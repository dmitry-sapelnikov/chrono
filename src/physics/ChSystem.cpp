//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010-2012 Alessandro Tasora
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

///////////////////////////////////////////////////
//
//   ChSystem.cpp
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

 
   
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <math.h>
#include <float.h>
#include <memory.h>
#include <algorithm>
 
#include "physics/ChSystem.h"
#include "physics/ChGlobal.h"
#include "physics/ChBodyAuxRef.h"
#include "physics/ChContactContainer.h"
#include "physics/ChProximityContainerBase.h"

#include "lcp/ChLcpSystemDescriptor.h"
#include "lcp/ChLcpSimplexSolver.h"
#include "lcp/ChLcpIterativeSOR.h"
#include "lcp/ChLcpIterativeSymmSOR.h"
#include "lcp/ChLcpIterativeSORmultithread.h"
#include "lcp/ChLcpIterativeJacobi.h"
//#include "lcp/ChLcpIterativeMINRES.h"
#include "lcp/ChLcpIterativePMINRES.h"
#include "lcp/ChLcpIterativeBB.h"
#include "lcp/ChLcpIterativePCG.h"
#include "lcp/ChLcpIterativeAPGD.h"
#include "lcp/ChLcpSolverDEM.h"
#include "parallel/ChOpenMP.h"

#include "core/ChTimer.h"
#include "collision/ChCCollisionSystemBullet.h"
#include "collision/ChCModelBulletBody.h"

#include "core/ChMemory.h" // must be last include (memory leak debugger). In .cpp only.


using namespace chrono::collision;

namespace chrono
{



// Hierarchy-handling functions

#define Bpointer		    (*ibody)
#define HIER_BODY_INIT      std::vector<ChBody*>::iterator ibody = bodylist.begin();
#define HIER_BODY_NOSTOP    (ibody != bodylist.end())
#define HIER_BODY_NEXT	    ibody++;

#define Lpointer		    (*iterlink)
#define HIER_LINK_INIT      std::list<ChLink*>::iterator iterlink = linklist.begin();
#define HIER_LINK_NOSTOP    (iterlink != linklist.end())
#define HIER_LINK_NEXT	    iterlink++;

#define PHpointer		    (*iterotherphysics)
#define HIER_OTHERPHYSICS_INIT    std::list<ChPhysicsItem*>::iterator iterotherphysics = otherphysicslist.begin();
#define HIER_OTHERPHYSICS_NOSTOP  (iterotherphysics != otherphysicslist.end())
#define HIER_OTHERPHYSICS_NEXT	  iterotherphysics++;

#define Ppointer		    (*iterprobe)
#define HIER_PROBE_INIT      std::vector<ChProbe*>::iterator iterprobe = probelist.begin();
#define HIER_PROBE_NOSTOP    (iterprobe != probelist.end())
#define HIER_PROBE_NEXT	    iterprobe++;

#define Cpointer		    (*itercontrol)
#define HIER_CONTROLS_INIT      std::vector<ChControls*>::iterator itercontrol = controlslist.begin();
#define HIER_CONTROLS_NOSTOP    (itercontrol != controlslist.end())
#define HIER_CONTROLS_NEXT	    itercontrol++;



/// Class for iterating through all items of ChPhysicalItem that exist in
/// a ChSystem. 
/// It will iterate through: 
/// - all ChBody          objects
/// - all ChLink          objects
/// - all ChPhysicalItems objects that were added, that were not ChBody or ChLink
/// - the ChContactContainer object that manages all the contacts.
/// Note that this iterator suffers a small overhead if compared to the use of
/// three iterator cycles in succession (one over ChBody list, one over ChLink list, etc.)
/// but it makes the code much more readable. Use it as follows:
///    IteratorAllPhysics my_iter(this);
///    while(my_iter.HasItem())
///    {
///        my_iter->....
///        ++my_iter;
///    }

class IteratorAllPhysics
{
public:
  IteratorAllPhysics(ChSystem* msys) : msystem(msys) 
  {
	  RewindToBegin();
  }
  ~IteratorAllPhysics() {}

  void RewindToBegin()
  {
	  list_bodies = msystem->Get_bodylist();
	  node_body   = list_bodies->begin();
	  list_links  = msystem->Get_linklist();
	  node_link	  = list_links->begin();
	  list_otherphysics  = msystem->Get_otherphysicslist();
	  node_otherphysics	 = list_otherphysics->begin();
	  stage = 0;
	  mptr = 0;
	  this->operator++(); // initialize with 1st available item
  }
  
  bool ReachedEnd()
  {
	  if (stage == 9999)
		  return true;
	  return false;
  }
  bool HasItem()
  {
	  if (mptr)
		  return true;
	  return false;
  }

  IteratorAllPhysics& operator=(const IteratorAllPhysics& other)
  {
	  msystem = other.msystem;
     //...***TO DO***
     return(*this);
  }

  bool operator==(const IteratorAllPhysics& other)
  {
     return(msystem == other.msystem); //...***TO COMPLETE***
  }

  bool operator!=(const IteratorAllPhysics& other)
  {
     return!(msystem == other.msystem); 
  }

  IteratorAllPhysics& operator++()
  {
	  switch (stage)
	  {
		case 1:
		{
			node_body++; // try next body
			if (node_body != list_bodies->end())
			{
				mptr = (*node_body); 
				return (*this);
			} 
			break;
		}
		case 2:
		{
			node_link++; // try next link
			if (node_link != list_links->end())
			{
				mptr = (*node_link); 
				return (*this);
			}
			break;
		}
		case 3:
		{
			node_otherphysics++; // try next otherphysics
			if (node_otherphysics != list_otherphysics->end())
			{
				mptr = (*node_otherphysics); 
				return (*this);
			}
			break;
		}
		default:
			break;
	  }
	  // Something went wrong, some list was at the end, so jump to beginning of next list
	  do
	  {
		  switch(stage)
		  {
		  case 0:
			  {
					  stage = 1;
					  if (node_body != list_bodies->end())
					  {
						  mptr = (*node_body); 
						  return (*this);
					  } 
					  break;
			  }
		  case 1:
			  {
					  stage = 2;
					  if (node_link != list_links->end())
					  {
						  mptr = (*node_link); 
						  return (*this);
					  } 
					  break;
			  }
		  case 2:
			  {
					  stage = 3;
					  if (node_otherphysics != list_otherphysics->end())
					  {
						  mptr = (*node_otherphysics); 
						  return (*this);
					  } 
					  break;
			  }
		  case 3:
			  {
					  stage = 4;
					  mptr = msystem->GetContactContainer(); 
					  return (*this);
			  }
		  case 4:
			  {
					  stage = 9999;
					  mptr = 0; 
					  return (*this);
			  }
		  } // end cases
	  } while (true);

     return(*this);
  }

  ChPhysicsItem* operator->()
  {
	  return (mptr);
  }
  ChPhysicsItem* operator*()
  {
	  return (mptr);
  }

private:
   std::vector<ChBody*>::iterator node_body;
   std::vector<ChBody*>* list_bodies;
   std::list<ChLink*>::iterator node_link;
   std::list<ChLink*>* list_links;
   std::list<ChPhysicsItem*>::iterator node_otherphysics;
   std::list<ChPhysicsItem*>* list_otherphysics;
   ChPhysicsItem* mptr;
   int stage;
   ChSystem* msystem;
};




//////////////////////////////////////
//////////////////////////////////////
// CLASS FOR PHYSICAL SYSTEM

// Register into the object factory, to enable run-time
// dynamic creation and persistence
ChClassRegister<ChSystem> a_registration_ChSystem;


ChSystem::ChSystem(unsigned int max_objects, double scene_size, bool init_sys)
{
	linklist.clear(); 
	bodylist.clear();
	otherphysicslist.clear();
	probelist.clear();
	controlslist.clear();
	
	
	nbodies=0;
	nlinks=0;
	nphysicsitems=0;
	ndof=0;
	ndoc=0;
	ndoc_w=0;
	ndoc_w_C=0;
	ndoc_w_D=0;
	nsysvars_w = 0;
	ncoords=0;
	ncoords_w=0;
	nsysvars = 0;
	ncoords_w=0;
	ncontacts = 0;
	nbodies_sleep=0;
	nbodies_fixed=0;

				// ------ PREFERENCES INITIALIZATION :
	ChTime = 0;

	end_time = 1;
	step = 0.04;
	step_min = 0.002;
	step_max = 0.04;
	tol = 0.0002;
	tol_speeds = 1e-6;
	normtype = NORM_INF;
	maxiter = 6;
	order = 1;
	SetMultisteps(1);
	dynaclose = 1;
	ns_close_pos = 1;
	ns_close_speed = 3;
	predict = FALSE;
	SetPredorder(2);
	dynatol = 10;
	stifftol = 3.2;
	monolat_tol = 2.5;
	integr_tol = 1.0;
	adaption  = STEP_FIXED;
	SetIntegrationType (INT_ANITESCU);
	modeXY = FALSE;
	auto_assembly = FALSE;
	contact_container = 0;

	msteps_collide = 6;
	min_bounce_speed = 0.15;
	max_penetration_recovery_speed = 0.6;

	parallel_thread_number = CHOMPfunctions::GetNumProcs(); // default n.threads as n.cores

	this->contact_container=0;
	// default contact container
	if(init_sys){
		this->contact_container = new ChContactContainer();
	}
	collision_system=0;
	// default GPU collision engine
	if(init_sys){
		collision_system = new ChCollisionSystemBullet(max_objects, scene_size);
	}
	

	LCP_descriptor = 0;
	LCP_solver_speed = 0;
	LCP_solver_stab = 0;

	iterLCPmaxIters = 30;
	iterLCPmaxItersStab = 10;
	simplexLCPmaxSteps = 100;
	if(init_sys){
		SetLcpSolverType(LCP_ITERATIVE_SYMMSOR);
	}
	
	use_GPU = false;
	use_sleeping = false;

	collision_callback = 0;
	collisionpoint_callback = 0;

	err_integr = 0.0;
	err_constr = 0.0;

	Set_G_acc (ChVector<>(0, -9.8, 0));
 
	stepcount = 0;

	last_err = 0;
	strcpy (err_message, "");
 
	scriptEngine = 0;
	scriptForStart = NULL;
	scriptForUpdate = NULL;
	scriptForStep = NULL;
	scriptFor3DStep = NULL;
	strcpy (scriptForStartFile, "");
	strcpy (scriptForUpdateFile, "");
	strcpy (scriptForStepFile, "");
	strcpy (scriptFor3DStepFile, "");

	events = new ChEvents(250);

	timer_step = 0;
	timer_lcp = 0;
	timer_collision_broad = 0;
	timer_collision_narrow = 0;
	timer_update = 0;
}


ChSystem::~ChSystem()
{

	RemoveAllBodies(); 
	RemoveAllLinks();
	RemoveAllOtherPhysicsItems();
	RemoveAllProbes();
	RemoveAllControls();

	if (LCP_solver_speed) delete LCP_solver_speed; LCP_solver_speed=0;
	if (LCP_solver_stab)  delete LCP_solver_stab;  LCP_solver_stab=0;
	if (LCP_descriptor) delete LCP_descriptor; LCP_descriptor=0;
	
	if (collision_system) delete collision_system; collision_system = 0;
	if (contact_container) delete contact_container; contact_container = 0;

	if (events) delete events; events = 0;

	if (scriptForStart)	 delete scriptForStart;
	if (scriptForUpdate) delete scriptForUpdate;
	if (scriptForStep)   delete scriptForStep;
	if (scriptFor3DStep) delete scriptFor3DStep;
}

void ChSystem::Copy(ChSystem* source)
{
	// first copy the parent class data...
	ChObj::Copy(source);

	G_acc = source->Get_G_acc();
	end_time = source->GetEndTime();
	step = source->GetStep();
	step_min = source->GetStepMin();
	step_max = source->GetStepMax();
	SetIntegrationType (source->GetIntegrationType());
	order = source->GetOrder();
	multisteps = source->GetMultisteps();
	tol = source->GetTol();
	tol_speeds = source->tol_speeds;
	normtype = source->GetNormType();
	maxiter = source->GetMaxiter();
	adaption = source->GetAdaption();
	nbodies = source->GetNbodies();
	nlinks = source->GetNlinks();
	nphysicsitems = source->GetNphysicsItems();
	ncoords = source->GetNcoords();
	ncoords_w = source->GetNcoords_w();
	ndoc = source->GetNdoc();
	ndoc_w = source->GetNdoc_w();
	ndoc_w_C = source->GetNdoc_w_C();
	ndoc_w_D = source->GetNdoc_w_D();
	ndof = source->GetNdof();
	nsysvars = source->GetNsysvars();
	nsysvars_w = source->GetNsysvars_w();
	ncontacts = source->GetNcontacts();
	nbodies_sleep = source->GetNbodiesSleeping();
	nbodies_fixed = source->GetNbodiesFixed();
	dynaclose = source->GetDynaclose();
	ns_close_pos = source->GetNsClosePos();
	ns_close_speed = source->GetNsCloseSpeed();
	dynatol = source->GetDynatol();
	monolat_tol = source->GetMonolattol();
	integr_tol = source->GetIntegrtol();
	predict = source->GetPredict();
	predorder = source->GetPredorder();
	stifftol = source->GetStifftol();
	modeXY = source->modeXY;
	err_integr = err_constr = 0.0;
	msteps_collide = source->msteps_collide;
	min_bounce_speed = source->min_bounce_speed;
	max_penetration_recovery_speed = source->max_penetration_recovery_speed;
	iterLCPmaxIters = source->iterLCPmaxIters;
	iterLCPmaxItersStab = source->iterLCPmaxItersStab;
	simplexLCPmaxSteps = source->simplexLCPmaxSteps;
	SetLcpSolverType(GetLcpSolverType());
	parallel_thread_number = source->parallel_thread_number;
	use_GPU = source->use_GPU;
	use_sleeping = source->use_sleeping;
	timer_step = source->timer_step;
	timer_lcp = source->timer_lcp;
	timer_collision_broad = source->timer_collision_broad;
	timer_collision_narrow = source->timer_collision_narrow;
	timer_update = source->timer_update;

	collision_callback = source->collision_callback;
	collisionpoint_callback = source->collisionpoint_callback;

	last_err = source->last_err;
	memcpy (err_message, source->err_message, (sizeof(char)*CHSYS_ERRLEN));

	RemoveAllLinks(); 
	RemoveAllBodies(); 
	RemoveAllProbes();
	RemoveAllControls();

	events->ResetAllEvents(); // don't copy events.


	SetScriptForStartFile(source->scriptForStartFile);
	SetScriptForUpdateFile(source->scriptForUpdateFile);
	SetScriptForStepFile(source->scriptForStepFile);
	SetScriptFor3DStepFile(source->scriptFor3DStepFile);

	ChTime = source->ChTime;
}

void ChSystem::Clear()
{

	nbodies=0;
	nlinks=0;
	nphysicsitems=0;
	ndof=0;
	ndoc=0;
	ndoc_w=0;
	ndoc_w_C=0;
	ndoc_w_D=0;
	nsysvars_w=0;
	ncoords=0;
	ncoords_w=0;
	nsysvars = 0;
	ncoords_w=0;
	ncontacts = 0;
	nbodies_sleep = 0;
	nbodies_fixed = 0;

	events->ResetAllEvents();

	//contact_container->RemoveAllContacts();

	RemoveAllLinks();
	RemoveAllBodies();
	RemoveAllOtherPhysicsItems();
	RemoveAllProbes();
	RemoveAllControls();

	//ResetTimers();
}

//
// Set/Get routines
//

void ChSystem::SetLcpSolverType(eCh_lcpSolver mval)
{
	lcp_solver_type = mval;

	use_GPU = false;

	if (LCP_solver_speed) delete LCP_solver_speed; LCP_solver_speed=0;
	if (LCP_solver_stab)  delete LCP_solver_stab;  LCP_solver_stab=0;
	if (LCP_descriptor) delete LCP_descriptor; LCP_descriptor=0;
	if (contact_container) delete contact_container; contact_container=0;

	LCP_descriptor = new ChLcpSystemDescriptor;
	LCP_descriptor->SetNumThreads(parallel_thread_number);

	contact_container = new ChContactContainer;
	

	switch (mval)
	{
	case LCP_ITERATIVE_SOR:  
		LCP_solver_speed = new ChLcpIterativeSOR(); 
		LCP_solver_stab  = new ChLcpIterativeSOR();
		break;
	case LCP_ITERATIVE_SYMMSOR:
		LCP_solver_speed = new ChLcpIterativeSymmSOR();
		LCP_solver_stab  = new ChLcpIterativeSymmSOR();
		break; 
	case LCP_SIMPLEX:
		LCP_solver_speed = new ChLcpSimplexSolver();
		LCP_solver_stab  = new ChLcpSimplexSolver();
		break;
	case LCP_ITERATIVE_JACOBI:
		LCP_solver_speed = new ChLcpIterativeJacobi();
		LCP_solver_stab  = new ChLcpIterativeJacobi();
		break;
	case LCP_ITERATIVE_SOR_MULTITHREAD:
		LCP_solver_speed = new ChLcpIterativeSORmultithread((char*)"speedLCP",parallel_thread_number);
		LCP_solver_stab = new ChLcpIterativeSORmultithread((char*)"posLCP",parallel_thread_number);
		break;
	case LCP_ITERATIVE_PMINRES: 
		LCP_solver_speed = new ChLcpIterativePMINRES();
		LCP_solver_stab = new ChLcpIterativePMINRES();
		break;
	case LCP_ITERATIVE_BARZILAIBORWEIN:
		LCP_solver_speed = new ChLcpIterativeBB();
		LCP_solver_stab = new ChLcpIterativeBB();
		break;
	case LCP_ITERATIVE_PCG:
		LCP_solver_speed = new ChLcpIterativePCG();
		LCP_solver_stab = new ChLcpIterativePCG();
		break;
	case LCP_ITERATIVE_APGD:
		LCP_solver_speed = new ChLcpIterativeAPGD();
		LCP_solver_stab = new ChLcpIterativeAPGD();
		break;
	case LCP_DEM:
		LCP_solver_speed = new ChLcpSolverDEM();
		LCP_solver_stab = new ChLcpSolverDEM();
		break;
	default:
		LCP_solver_speed = new ChLcpIterativeSymmSOR();
		LCP_solver_stab  = new ChLcpIterativeSymmSOR();
		break;
	} 
}  


ChLcpSolver* ChSystem::GetLcpSolverSpeed()
{
	// in case the solver is iterative, pre-configure it with max.iter.number
	switch(lcp_solver_type)
	{
	case LCP_DEM:
		break;
	case LCP_SIMPLEX:
		((ChLcpSimplexSolver*)LCP_solver_speed)->SetTruncationStep(GetSimplexLCPmaxSteps());
		break;
	default:
		ChLcpIterativeSolver* iter_solver = (ChLcpIterativeSolver*)LCP_solver_speed;
		iter_solver->SetMaxIterations(GetIterLCPmaxItersSpeed());
		iter_solver->SetTolerance(this->tol_speeds); 
		break;
	}
	/*
	if (lcp_solver_type != LCP_SIMPLEX)
	{
		ChLcpIterativeSolver* iter_solver = (ChLcpIterativeSolver*)LCP_solver_speed;
		iter_solver->SetMaxIterations(GetIterLCPmaxItersSpeed());
		iter_solver->SetTolerance(this->tol_speeds); 
	}
	else 
	{ 
		ChLcpSimplexSolver* simplex_solver = (ChLcpSimplexSolver*)LCP_solver_speed;
		simplex_solver->SetTruncationStep(GetSimplexLCPmaxSteps());
	}
	*/
	return LCP_solver_speed;
}

ChLcpSolver* ChSystem::GetLcpSolverStab()
{
	// in case the solver is iterative, pre-configure it with max.iter.number
	if (lcp_solver_type != LCP_SIMPLEX)
	{
		ChLcpIterativeSolver* iter_solver = (ChLcpIterativeSolver*)LCP_solver_stab;
		iter_solver->SetMaxIterations(GetIterLCPmaxItersStab());
		iter_solver->SetTolerance(this->tol);
	}
	else
	{
		ChLcpSimplexSolver* simplex_solver = (ChLcpSimplexSolver*)LCP_solver_stab;
		simplex_solver->SetTruncationStep(GetSimplexLCPmaxSteps());
	}
	return LCP_solver_stab;
}

void ChSystem::SetIterLCPwarmStarting(bool usewarm)
{
	if (ChLcpIterativeSolver* iter_solver_speed = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_speed))
	{
		iter_solver_speed->SetWarmStart(usewarm);
	}
	if (ChLcpIterativeSolver* iter_solver_stab = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_stab))
	{
		iter_solver_stab->SetWarmStart(usewarm);
	}
}

bool ChSystem::GetIterLCPwarmStarting()
{
	if (ChLcpIterativeSolver* iter_solver_speed = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_speed))
	{
		return iter_solver_speed->GetWarmStart();
	}
	return false;
}

void ChSystem::SetIterLCPomega(double momega)
{
	if (ChLcpIterativeSolver* iter_solver_speed = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_speed))
	{
		iter_solver_speed->SetOmega(momega);
	}
	if (ChLcpIterativeSolver* iter_solver_stab = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_stab))
	{
		iter_solver_stab->SetOmega(momega);
	}
}

double ChSystem::GetIterLCPomega()
{
	if (ChLcpIterativeSolver* iter_solver_speed = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_speed))
	{
		return iter_solver_speed->GetOmega();
	}
	return 1.0;
}

void ChSystem::SetIterLCPsharpnessLambda(double momega)
{
	if (ChLcpIterativeSolver* iter_solver_speed = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_speed))
	{
		iter_solver_speed->SetSharpnessLambda(momega);
	}
	if (ChLcpIterativeSolver* iter_solver_stab = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_stab))
	{
		iter_solver_stab->SetSharpnessLambda(momega);
	}
}

double ChSystem::GetIterLCPsharpnessLambda()
{
	if (ChLcpIterativeSolver* iter_solver_speed = dynamic_cast<ChLcpIterativeSolver*>(LCP_solver_speed))
	{
		return iter_solver_speed->GetSharpnessLambda();
	}
	return 1.0;
}


void ChSystem::SetParallelThreadNumber(int mthreads)
{
	if (mthreads<1) 
		mthreads =1;

	parallel_thread_number = mthreads;
	
	LCP_descriptor->SetNumThreads(mthreads);

	if (lcp_solver_type == LCP_ITERATIVE_SOR_MULTITHREAD)
	{
		((ChLcpIterativeSORmultithread*)LCP_solver_speed)->ChangeNumberOfThreads(mthreads);
		((ChLcpIterativeSORmultithread*)LCP_solver_stab)->ChangeNumberOfThreads(mthreads);
	}
}




// Plug-in components configuration


void ChSystem::ChangeLcpSystemDescriptor(ChLcpSystemDescriptor* newdescriptor)
{
	assert (newdescriptor);
	if (this->LCP_descriptor) 
		delete (this->LCP_descriptor);
	this->LCP_descriptor = newdescriptor;
}
void ChSystem::ChangeLcpSolverSpeed(ChLcpSolver* newsolver)
{
	assert (newsolver);
	if (this->LCP_solver_speed) 
		delete (this->LCP_solver_speed);
	this->LCP_solver_speed = newsolver;
}

void ChSystem::ChangeLcpSolverStab(ChLcpSolver* newsolver)
{
	assert (newsolver);
	if (this->LCP_solver_stab) 
		delete (this->LCP_solver_stab);
	this->LCP_solver_stab = newsolver;
} 

void ChSystem::ChangeContactContainer(ChContactContainerBase* newcontainer)
{
	assert (newcontainer);
	if (this->contact_container) 
		delete (this->contact_container);
	this->contact_container = newcontainer;
}

void ChSystem::ChangeCollisionSystem(ChCollisionSystem* newcollsystem)
{
	assert (this->GetNbodies()==0);
	assert (newcollsystem);
	if (this->collision_system) 
		delete (this->collision_system);
	this->collision_system = newcollsystem;
}

// JS commands


int ChSystem::SetScriptForStartFile(char* mfile)
{
	if (!this->scriptEngine) return 0;
	strcpy (this->scriptForStartFile, mfile);
	this->scriptForStart = this->scriptEngine->CreateScript();
	return this->scriptEngine->FileToScript(*this->scriptForStart, mfile);
}
int ChSystem::SetScriptForUpdateFile(char* mfile)
{
	if (!this->scriptEngine) return 0;
	strcpy (this->scriptForUpdateFile, mfile);
	this->scriptForUpdate = this->scriptEngine->CreateScript();
	return this->scriptEngine->FileToScript(*this->scriptForUpdate, mfile);
}
int ChSystem::SetScriptForStepFile(char* mfile)
{
	if (!this->scriptEngine) return 0;
	strcpy (this->scriptForStepFile, mfile);
	this->scriptForStep = this->scriptEngine->CreateScript();
	return this->scriptEngine->FileToScript(*this->scriptForStep, mfile);
}
int ChSystem::SetScriptFor3DStepFile(char* mfile)
{
	if (!this->scriptEngine) return 0;
	strcpy (this->scriptFor3DStepFile, mfile);
	this->scriptFor3DStep = this->scriptEngine->CreateScript();
	return this->scriptEngine->FileToScript(*this->scriptFor3DStep, mfile);
}

int ChSystem::ExecuteScriptForStart()
{
	if (!this->scriptEngine) return 0;
	return this->scriptEngine->ExecuteScript(*this->scriptForStart);
}
int ChSystem::ExecuteScriptForUpdate()
{
	if (!this->scriptEngine) return 0;
	return this->scriptEngine->ExecuteScript(*this->scriptForUpdate);
}
int ChSystem::ExecuteScriptForStep()
{
	if (!this->scriptEngine) return 0;
	return this->scriptEngine->ExecuteScript(*this->scriptForStep);
}
int ChSystem::ExecuteScriptFor3DStep()
{
	if (!this->scriptEngine) return 0;
	return this->scriptEngine->ExecuteScript(*this->scriptFor3DStep);
}


// PROBE STUFF

int ChSystem::RecordAllProbes()
{
	int pcount = 0;

	HIER_PROBE_INIT
	while HIER_PROBE_NOSTOP
	{
		Ppointer->Record(this->GetChTime());

		HIER_PROBE_NEXT
	}

	return pcount;
}

int ChSystem::ResetAllProbes()
{
	int pcount = 0;

	HIER_PROBE_INIT
	while HIER_PROBE_NOSTOP
	{
		Ppointer->Reset();

		HIER_PROBE_NEXT
	}

	return pcount;
}

// CONTROLS STUFF


int ChSystem::ExecuteControlsForStart()
{
	HIER_CONTROLS_INIT
	while HIER_CONTROLS_NOSTOP
	{
		Cpointer->ExecuteForStart();
		HIER_CONTROLS_NEXT
	}
	return TRUE;
}

int ChSystem::ExecuteControlsForUpdate()
{
	HIER_CONTROLS_INIT
	while HIER_CONTROLS_NOSTOP
	{
		Cpointer->ExecuteForUpdate();
		HIER_CONTROLS_NEXT
	}
	return TRUE;
}

int ChSystem::ExecuteControlsForStep()
{
	HIER_CONTROLS_INIT
	while HIER_CONTROLS_NOSTOP
	{
		Cpointer->ExecuteForStep();
		HIER_CONTROLS_NEXT
	}
	return TRUE;
}

int ChSystem::ExecuteControlsFor3DStep()
{
	HIER_CONTROLS_INIT
	while HIER_CONTROLS_NOSTOP
	{
		Cpointer->ExecuteFor3DStep();
		HIER_CONTROLS_NEXT
	}
	return TRUE;
}




// 
// HIERARCHY HANDLERS
//



void ChSystem::AddBody (ChSharedPtr<ChBody> newbody)
{
	assert(std::find<std::vector<ChBody*>::iterator>(bodylist.begin(), bodylist.end(), newbody.get_ptr())==bodylist.end());
	assert(newbody->GetSystem()==0); // should remove from other system before adding here

	newbody->AddRef();
	newbody->SetSystem (this);
	bodylist.push_back((newbody).get_ptr());

	// add to collision system too
	if (newbody->GetCollide())
		newbody->AddCollisionModelsToSystem(); 
}

void ChSystem::RemoveBody (ChSharedPtr<ChBody> mbody)
{
	assert(std::find<std::vector<ChBody*>::iterator>(bodylist.begin(), bodylist.end(), mbody.get_ptr() )!=bodylist.end());

	// remove from collision system
	if (mbody->GetCollide())
		mbody->RemoveCollisionModelsFromSystem(); 
 
	// warning! linear time search, to erase pointer from container.
	bodylist.erase(std::find<std::vector<ChBody*>::iterator>(bodylist.begin(), bodylist.end(), mbody.get_ptr() ) );
	
	// nullify backward link to system
	mbody->SetSystem(0);
	// this may delete the body, if none else's still referencing it..
	mbody->RemoveRef();
}
   
void ChSystem::AddLink (ChLink* newlink)
{ 
	assert(std::find<std::list<ChLink*>::iterator>(linklist.begin(), linklist.end(), newlink)==linklist.end());

	newlink->AddRef();
	newlink->SetSystem (this);
	linklist.push_back(newlink);
}

void ChSystem::AddLink (ChSharedPtr<ChLink> newlink)
{
	AddLink(newlink.get_ptr());
}

// Faster than RemoveLink because it does not require the linear time search
std::list<ChLink*>::iterator ChSystem::RemoveLinkIter(std::list<ChLink*>::iterator& mlinkiter)
{
	// nullify backward link to system
	(*mlinkiter)->SetSystem(0);
	// this may delete the link, if none else's still referencing it..
	(*mlinkiter)->RemoveRef();

	return linklist.erase(mlinkiter);
}

void ChSystem::RemoveLink (ChSharedPtr<ChLink> mlink)
{
	assert(std::find<std::list<ChLink*>::iterator>(linklist.begin(), linklist.end(), mlink.get_ptr() )!=linklist.end());

	// warning! linear time search, to erase pointer from container.
	linklist.remove(mlink.get_ptr());//erase(std::find<std::vector<ChBody*>::iterator>(bodylist.begin(), bodylist.end(), mbody.get_ptr() ) );
	
	// nullify backward link to system
	mlink->SetSystem(0);
	// this may delete the body, if none else's still referencing it..
	mlink->RemoveRef();
}


void ChSystem::AddOtherPhysicsItem (ChSharedPtr<ChPhysicsItem> newitem)
{
	assert(std::find<std::list<ChPhysicsItem*>::iterator>(otherphysicslist.begin(), otherphysicslist.end(), newitem.get_ptr())==otherphysicslist.end());
	//assert(newitem->GetSystem()==0); // should remove from other system before adding here

	newitem->AddRef();
	newitem->SetSystem (this);
	otherphysicslist.push_back((newitem).get_ptr());

	// add to collision system too
	if (newitem->GetCollide())
		newitem->AddCollisionModelsToSystem(); 
}

void ChSystem::RemoveOtherPhysicsItem (ChSharedPtr<ChPhysicsItem> mitem)
{
	assert(std::find<std::list<ChPhysicsItem*>::iterator>(otherphysicslist.begin(), otherphysicslist.end(), mitem.get_ptr())!=otherphysicslist.end());

	// remove from collision system
	if (mitem->GetCollide())
		mitem->RemoveCollisionModelsFromSystem();
 
	// warning! linear time search, to erase pointer from container.
	otherphysicslist.erase(std::find<std::list<ChPhysicsItem*>::iterator>(otherphysicslist.begin(), otherphysicslist.end(), mitem.get_ptr() ) );
	
	// nullify backward link to system
	mitem->SetSystem(0);
	// this may delete the body, if none else's still referencing it..
	mitem->RemoveRef();
}

void ChSystem::Add (ChSharedPtr<ChPhysicsItem> newitem)
{
	if (typeid(*newitem.get_ptr())==typeid(ChBody))
	{
		AddBody((ChSharedPtr<ChBody>)newitem);
	}else
		if (dynamic_cast<ChLink*>(newitem.get_ptr()))
		{
			AddLink((ChSharedPtr<ChLink>)newitem);
		}else
			  AddOtherPhysicsItem(newitem);
}

void ChSystem::Remove (ChSharedPtr<ChPhysicsItem> newitem)
{
	if (typeid(*newitem.get_ptr())==typeid(ChBody))
	{
		RemoveBody((ChSharedPtr<ChBody>)newitem);
	}else
		if (dynamic_cast<ChLink*>(newitem.get_ptr()))
		{
			RemoveLink((ChSharedPtr<ChLink>)newitem);
		}else
			  RemoveOtherPhysicsItem(newitem);
}



void ChSystem::AddProbe (ChSharedPtr<ChProbe>& newprobe)
{
	assert(std::find<std::vector<ChProbe*>::iterator>(probelist.begin(), probelist.end(), newprobe.get_ptr())==probelist.end());

	newprobe->AddRef();
	//newprobe->SetSystem (this);
	probelist.push_back(newprobe.get_ptr());
}

void ChSystem::AddControls (ChSharedPtr<ChControls>& newcontrols)
{
	assert(std::find<std::vector<ChControls*>::iterator>(controlslist.begin(), controlslist.end(), newcontrols.get_ptr())==controlslist.end());

	newcontrols->AddRef();
	//newcontrols->SetSystem (this);
	controlslist.push_back(newcontrols.get_ptr());
}


   
void ChSystem::RemoveAllBodies() 
{ 
	HIER_BODY_INIT
	while (HIER_BODY_NOSTOP)
	{
		// remove from collision system
		if (Bpointer->GetCollide())
			Bpointer->RemoveCollisionModelsFromSystem(); 
		// nullify backward link to system
		Bpointer->SetSystem(0);	
		// this may delete the body, if none else's still referencing it..
		Bpointer->RemoveRef();
		HIER_BODY_NEXT
	}	
	bodylist.clear(); 
}; 


void ChSystem::RemoveAllLinks() 
{ 

	HIER_LINK_INIT
	while (HIER_LINK_NOSTOP)
	{
		// nullify backward link to system
		Lpointer->SetSystem(0);	
		// this may delete the link, if none else's still referencing it..
		Lpointer->RemoveRef();
		HIER_LINK_NEXT
	}	
	linklist.clear(); 
};

void ChSystem::RemoveAllOtherPhysicsItems() 
{ 
	HIER_OTHERPHYSICS_INIT
	while (HIER_OTHERPHYSICS_NOSTOP)
	{
		// remove from collision system
		if (PHpointer->GetCollide())
			PHpointer->RemoveCollisionModelsFromSystem();  
		// nullify backward link to system
		PHpointer->SetSystem(0);	
		// this may delete the item, if none else's still referencing it..
		PHpointer->RemoveRef();
		HIER_OTHERPHYSICS_NEXT
	}	
	otherphysicslist.clear(); 
}; 

void ChSystem::RemoveAllProbes() 
{ 
	HIER_PROBE_INIT
	while (HIER_PROBE_NOSTOP)
	{
		//Ppointer->SetSystem(0);	
		Ppointer->RemoveRef();
		HIER_PROBE_NEXT
	}	
	probelist.clear();
};

void ChSystem::RemoveAllControls() 
{ 
	HIER_CONTROLS_INIT
	while (HIER_CONTROLS_NOSTOP)
	{
		//Cpointer->SetSystem(0);	
		Cpointer->RemoveRef();
		HIER_CONTROLS_NEXT
	}	
	controlslist.clear();
};
 

 

ChSharedPtr<ChBody> ChSystem::SearchBody (char* m_name)
{
	ChBody* mbody = ChContainerSearchFromName<ChBody, std::vector<ChBody*>::iterator>
				(m_name, 
				bodylist.begin(), 
				bodylist.end());
	if (mbody)
	{
		mbody->AddRef(); // in that container pointers were not stored as ChSharedPtr, so this is needed..
		return (ChSharedPtr<ChBody>(mbody));  // ..here I am not getting a new() data, but a reference to something created elsewhere
	}
	return (ChSharedPtr<ChBody>()); // not found? return a void shared ptr.
}

ChSharedPtr<ChLink> ChSystem::SearchLink (char* m_name)
{
	ChLink* mlink = ChContainerSearchFromName<ChLink, std::list<ChLink*>::iterator>
				(m_name, 
				linklist.begin(), 
				linklist.end());
	if (mlink)
	{
		mlink->AddRef(); // in that container pointers were not stored as ChSharedPtr, so this is needed..
		return (ChSharedPtr<ChLink>(mlink));  // ..here I am not getting a new() data, but a reference to something created elsewhere
	}
	return (ChSharedPtr<ChLink>()); // not found? return a void shared ptr.
}

ChSharedPtr<ChPhysicsItem> ChSystem::SearchOtherPhysicsItem (char* m_name)
{
	ChPhysicsItem* mitem = ChContainerSearchFromName<ChPhysicsItem, std::list<ChPhysicsItem*>::iterator>
				(m_name, 
				otherphysicslist.begin(), 
				otherphysicslist.end());
	if (mitem)
	{
		mitem->AddRef(); // in that container pointers were not stored as ChSharedPtr, so this is needed..
		return (ChSharedPtr<ChPhysicsItem>(mitem));  // ..here I am not getting a new() data, but a reference to something created elsewhere
	}
	return (ChSharedPtr<ChPhysicsItem>()); // not found? return a void shared ptr.
}

ChSharedPtr<ChPhysicsItem> ChSystem::Search (char* m_name)
{
	ChSharedPtr<ChBody> mbo = SearchBody(m_name);
	if (!mbo.IsNull())
		return mbo;
	ChSharedPtr<ChLink> mli = SearchLink(m_name);
	if (!mli.IsNull())
		return mli;
	ChSharedPtr<ChPhysicsItem> mph = SearchOtherPhysicsItem(m_name);
	if (!mph.IsNull())
		return mph;
	return (ChSharedPtr<ChPhysicsItem>()); // not found? return a void shared ptr.
}


ChSharedPtr<ChMarker> ChSystem::SearchMarker (char* m_name)
{
	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		ChSharedPtr<ChMarker> mmark = Bpointer->SearchMarker(m_name);
		if (!mmark.IsNull())
			return mmark;
		HIER_BODY_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		if (ChBodyAuxRef* mbodyauxref = dynamic_cast<ChBodyAuxRef*>(PHpointer))
		{
			ChSharedPtr<ChMarker> mmark = mbodyauxref->SearchMarker(m_name);
			if (!mmark.IsNull())
				return mmark;
		}
		HIER_OTHERPHYSICS_NEXT
	}

	return (ChSharedPtr<ChMarker>()); // not found? return a void shared ptr.
}

ChSharedPtr<ChMarker> ChSystem::SearchMarker (int markID)
{
	ChMarker* candidate = NULL;
	ChMarker* res = NULL;

	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		res = ChContainerSearchFromID<ChMarker, std::vector<ChMarker*>::iterator>
				(markID, 
				Bpointer->GetMarkerList()->begin(), 
				Bpointer->GetMarkerList()->end());
		if (res != NULL) 
		{
			res->AddRef(); // in that container pointers were not stored as ChSharedPtr, so this is needed..
			return (ChSharedPtr<ChMarker>(res));  // ..here I am not getting a new() data, but a reference to something created elsewhere		
		}

		HIER_BODY_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		if (ChBodyAuxRef* mbodyauxref = dynamic_cast<ChBodyAuxRef*>(PHpointer))
		{
			res = ChContainerSearchFromID<ChMarker, std::vector<ChMarker*>::iterator>
				(markID, 
				mbodyauxref->GetMarkerList()->begin(), 
				mbodyauxref->GetMarkerList()->end());
			if (res != NULL) 
			{
				res->AddRef(); // in that container pointers were not stored as ChSharedPtr, so this is needed..
				return (ChSharedPtr<ChMarker>(res));  // ..here I am not getting a new() data, but a reference to something created elsewhere		
			}
		}
		HIER_OTHERPHYSICS_NEXT
	}

	return (ChSharedPtr<ChMarker>()); // not found? return a void shared ptr.
}







void ChSystem::Reference_LM_byID()
{
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		if (ChLinkMarkers* malink = ChDynamicCast(ChLinkMarkers,Lpointer))
		{
			ChSharedPtr<ChMarker> shm1 = SearchMarker(malink->GetMarkID1());
			ChSharedPtr<ChMarker> shm2 = SearchMarker(malink->GetMarkID2());
			ChMarker* mm1 = shm1.get_ptr();
			ChMarker* mm2 = shm1.get_ptr();
			malink->SetMarker1(mm1);
			malink->SetMarker2(mm2);
			if (mm1 && mm2)
			{
				Lpointer->SetValid(true);
				HIER_LINK_NEXT
			}
			else
			{
				Lpointer->SetValid(false);
				malink->SetMarkers(0,0); // however marker IDs will survive!!
				iterlink = RemoveLinkIter(iterlink); // may delete it...
			}
		}
		else
		{
			HIER_LINK_NEXT
		}
	}
}

//////
////// PREFERENCES


void ChSystem::SetIntegrationType (eCh_integrationType m_integration)
{									
	// set integration scheme:
	integration_type = m_integration;				
	
	// set stability region
	switch (integration_type)
	{
		case INT_ANITESCU:
			SetMultisteps (1);
			SetOrder (1);
		case INT_TASORA:
			SetMultisteps (1);
			SetOrder (1);
		default:
			SetMultisteps (1);
	}
	// ......... //
}





void ChSystem::WakeUpSleepingBodies()
{
	// Make this class for iterating through contacts (if supported by
	// contact container)

	class _wakeup_reporter_class : public ChReportContactCallback 
	{
	public:
				/// Callback, used to report contact points already added to the container.
				/// This must be implemented by a child class of ChReportContactCallback.
				/// If returns false, the contact scanning will be stopped.
		virtual bool ReportContactCallback (
						const ChVector<>& pA,				///< get contact pA
						const ChVector<>& pB,				///< get contact pB
						const ChMatrix33<>& plane_coord,	///< get contact plane coordsystem (A column 'X' is contact normal)
						const double& distance,				///< get contact distance
						const float& mfriction,			  	///< get friction info
						const ChVector<>& react_forces,		///< get react.forces (if already computed). In coordsystem 'plane_coord'
						const ChVector<>& react_torques,	///< get react.torques, if rolling friction (if already computed)
						collision::ChCollisionModel* modA,	///< get model A (note: some containers may not support it and could be zero!)
						collision::ChCollisionModel* modB	///< get model B (note: some containers may not support it and could be zero!)
											) 
		{
			if (!(modA && modB)) 
				return true;
			ChBody* b1=0;
			ChBody* b2=0;
			if (ChModelBulletBody* mmboA = dynamic_cast<ChModelBulletBody*>(modA))
				b1 = mmboA->GetBody();
			if (ChModelBulletBody* mmboB = dynamic_cast<ChModelBulletBody*>(modB))
				b2 = mmboB->GetBody();
			if (!(b1 && b2)) 
				return true;
			bool sleep1 = b1->GetSleeping();
			bool sleep2 = b2->GetSleeping();
			bool ground1 = b1->GetBodyFixed();
			bool ground2 = b2->GetBodyFixed();
			if (sleep1 && !sleep2 && !ground2)
			{
				b1->SetSleeping(false);
			}
			if (sleep2 && !sleep1 && !ground1)
			{
				b2->SetSleeping(false);
			}
			this->someone_sleeps = sleep1 | sleep2 | this->someone_sleeps;

			return true; // to continue scanning contacts
		}

		// Data 
		bool someone_sleeps;
	};

	_wakeup_reporter_class my_waker;

	if (this->GetUseSleeping())
	{
		for (int i=0; i<1; i++)  //***TO DO*** reconfigurable number of wakeup cycles
		{
			my_waker.someone_sleeps = false;

			// scan all links and wake connected bodies
			HIER_LINK_INIT
			while HIER_LINK_NOSTOP
			{
				if (Lpointer->IsRequiringWaking())
				{
					Lpointer->GetBody1()->SetSleeping(false);
					Lpointer->GetBody2()->SetSleeping(false);
				}
				HIER_LINK_NEXT
			}
			
			// scan all contacts and wake neighbouring bodies
			this->contact_container->ReportAllContacts(&my_waker);
			
			// bailout wakeup cycle prematurely, if all bodies are not sleeping
			if (!my_waker.someone_sleeps) 
				break; 
		}
	}
}




////////////////////////////////
//////
////// UPDATING ROUTINES
//////
//////


		// COUNT ALL BODIES AND LINKS, ETC, COMPUTE &SET DOF FOR STATISTICS,
		// ALLOCATES OR REALLOCATE BOOKKEEPING DATA/VECTORS, IF ANY

int ChSystem::Setup()
{
	events->Record(CHEVENT_SETUP);

	int need_update = FALSE;
	int old_ncoords  = ncoords ;
	int old_ndoc = ndoc;
	int old_ndoc_w_D = ndoc_w_D;
	
	nbodies = 0;
	nbodies_sleep = 0;
	nbodies_fixed = 0;
	ncoords=0;
	ncoords_w=0;
	ndoc =0;
	ndoc_w =0;
	ndoc_w_C = 0;
	ndoc_w_D = 0;
	nlinks = 0;
	nphysicsitems = 0;

							
	HIER_BODY_INIT					
	while HIER_BODY_NOSTOP		
	{
		if (!Bpointer->GetBodyFixed())
		{
			if (!Bpointer->GetSleeping())
			{
				nbodies ++;	// Count bodies and indicize them.
			}
			else
			{
				nbodies_sleep ++;
			}
		}
		else
		{
			nbodies_fixed ++;
		}
 
		HIER_BODY_NEXT
	}

	ncoords_w += nbodies * 6;
	ncoords   += nbodies * 7; // with quaternion coords
	ndoc	  += nbodies; // There is a quaternion constr. for each active body.


	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		nphysicsitems ++;	

		ncoords_w += PHpointer->GetDOF();
		ndoc_w	  += PHpointer->GetDOC();
		ndoc_w_C  += PHpointer->GetDOC_c();
		ndoc_w_D  += PHpointer->GetDOC_d();

		HIER_OTHERPHYSICS_NEXT
	}

	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		nlinks ++;

		ndoc_w   += Lpointer->GetDOC();
		ndoc_w_C += Lpointer->GetDOC_c();
		ndoc_w_D += Lpointer->GetDOC_d();

		HIER_LINK_NEXT
	}

	ndoc_w_D += contact_container->GetDOC_d();


	ndoc = ndoc_w + nbodies;		// sets number of constraints including quaternion constraints.
	nsysvars   = ncoords   + ndoc;  // sets number of total variables (=coordinates + lagrangian multipliers)
	nsysvars_w = ncoords_w + ndoc_w;// sets number of total variables (with 6 dof per body)

	ndof= ncoords - ndoc;			// sets number of left degrees of freedom (approximate - does not consider constr. redundancy, etc)

	return need_update;
}



		// - ALL PHYSICAL ITEMS (BODIES, LINKS,ETC.) ARE UPDATED,
		//   ALSO UPDATING THEIR AUXILIARY VARIABLES (ROT.MATRICES, ETC.).
		// - UPDATES ALL FORCES  (AUTOMATIC, AS CHILDREN OF BODIES)
		// - UPDATES ALL MARKERS (AUTOMATIC, AS CHILDREN OF BODIES).

void ChSystem::Update() 
{

	ChTimer<double>mtimer; mtimer.start(); // Timer for profiling


	events->Record(CHEVENT_UPDATE); // Record an update event

									// Executes the "forUpdate" script, if any
	ExecuteScriptForUpdate();
									// Executes the "forUpdate" script
									// in all controls of controlslist
	ExecuteControlsForUpdate();

									// --------------------------------------
									// Spread state vector Y to bodies
									//    Y --> Bodies
	HIER_BODY_INIT					//    Y_accel --> Bodies
	while HIER_BODY_NOSTOP			// Updates recursively all other aux.vars
	{								// --------------------------------------
		Bpointer->Update(ChTime);  

		if (this->GetUseSleeping())
			Bpointer->TrySleeping();

		HIER_BODY_NEXT
	}
									// -----------------------------
									// Updates other physical items
									// -----------------------------
	HIER_OTHERPHYSICS_INIT					
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->Update(ChTime);

		HIER_OTHERPHYSICS_NEXT
	}
									// -----------------------------
									// Updates all links
									// -----------------------------
	HIER_LINK_INIT					
	while HIER_LINK_NOSTOP
	{
		Lpointer->Update(ChTime);

		HIER_LINK_NEXT
	}

	this->contact_container->Update(); // Update all contacts, if any

	mtimer.stop();
	timer_update += mtimer();
}




void ChSystem::UpdateExternalGeometry ()
{
	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		Bpointer->UpdateExternalGeometry ();

		HIER_BODY_NEXT
	}
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->UpdateExternalGeometry ();

		HIER_LINK_NEXT
	}
}




///////////////////////////////
/////////
/////////   SPARSE LCP BOOKKEEPING
/////////


void ChSystem::LCPprepare_reset()
{
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->ConstraintsBiReset();
		HIER_LINK_NEXT
	}
	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		Bpointer->VariablesFbReset();
		HIER_BODY_NEXT
	}	
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->VariablesFbReset();
		PHpointer->ConstraintsBiReset();
		HIER_OTHERPHYSICS_NEXT
	}
	this->contact_container->ConstraintsBiReset();
}


void ChSystem::LCPprepare_load(bool load_jacobians,
							   bool load_Mv,
							   double F_factor,
							   double K_factor,		
							   double R_factor,
							   double M_factor,
							   double Ct_factor,
							   double C_factor,
							   double recovery_clamp,
							   bool do_clamp
							    )
{
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		if (C_factor)
			Lpointer->ConstraintsBiLoad_C(C_factor, recovery_clamp, do_clamp);
		if (Ct_factor)
			Lpointer->ConstraintsBiLoad_Ct(Ct_factor);			// Ct
		if (F_factor)
			Lpointer->ConstraintsFbLoadForces(F_factor);		// f*dt
		if (load_Mv)
		{
			Lpointer->VariablesQbLoadSpeed();					//   v_old 
			Lpointer->VariablesFbIncrementMq();					// M*v_old
		}
		if (load_jacobians)
			Lpointer->ConstraintsLoadJacobians();
		HIER_LINK_NEXT
	}

	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		if (F_factor)
			Bpointer->VariablesFbLoadForces(F_factor);			// f*dt
		if (load_Mv)
		{
			Bpointer->VariablesQbLoadSpeed();					//   v_old 
			Bpointer->VariablesFbIncrementMq();					// M*v_old
		} 
		HIER_BODY_NEXT
	}

	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		if (F_factor)
			PHpointer->VariablesFbLoadForces(F_factor);			// f*dt
		if (load_Mv)
		{
			PHpointer->VariablesQbLoadSpeed();					//   v_old 
			PHpointer->VariablesFbIncrementMq();				// M*v_old
		}
		if (C_factor)
			PHpointer->ConstraintsBiLoad_C(C_factor, recovery_clamp, do_clamp);
		if (Ct_factor)
			PHpointer->ConstraintsBiLoad_Ct(Ct_factor);			// Ct
		if (load_jacobians)
			PHpointer->ConstraintsLoadJacobians();
		if (K_factor || R_factor || M_factor)
			PHpointer->KRMmatricesLoad(K_factor, R_factor, M_factor);
		HIER_OTHERPHYSICS_NEXT
	}

	if (C_factor)
		contact_container->ConstraintsBiLoad_C(C_factor, recovery_clamp, do_clamp);
	if (F_factor)
		contact_container->ConstraintsFbLoadForces(F_factor);		// f*dt
	if (load_jacobians)
		contact_container->ConstraintsLoadJacobians();

} 

void ChSystem::LCPprepare_inject(ChLcpSystemDescriptor& mdescriptor)
{
	mdescriptor.BeginInsertion(); // This resets the vectors of constr. and var. pointers.

	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->InjectConstraints(mdescriptor);
		HIER_LINK_NEXT
	}
	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		Bpointer->InjectVariables(mdescriptor);
		HIER_BODY_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->InjectVariables(mdescriptor);
		PHpointer->InjectConstraints(mdescriptor);
		PHpointer->InjectKRMmatrices(mdescriptor);
		HIER_OTHERPHYSICS_NEXT
	}
	this->contact_container->InjectConstraints(mdescriptor);

	mdescriptor.EndInsertion(); 
}


void ChSystem::LCPprepare_Li_from_speed_cache()
{
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->ConstraintsLiLoadSuggestedSpeedSolution();
		HIER_LINK_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->ConstraintsLiLoadSuggestedSpeedSolution();
		HIER_OTHERPHYSICS_NEXT
	}
	this->contact_container->ConstraintsLiLoadSuggestedSpeedSolution();
}

void ChSystem::LCPprepare_Li_from_position_cache()
{
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->ConstraintsLiLoadSuggestedPositionSolution();
		HIER_LINK_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->ConstraintsLiLoadSuggestedPositionSolution();
		HIER_OTHERPHYSICS_NEXT
	}
	this->contact_container->ConstraintsLiLoadSuggestedPositionSolution();
}

void ChSystem::LCPresult_Li_into_speed_cache()
{
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->ConstraintsLiFetchSuggestedSpeedSolution();
		HIER_LINK_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->ConstraintsLiFetchSuggestedSpeedSolution();
		HIER_OTHERPHYSICS_NEXT
	}
	this->contact_container->ConstraintsLiFetchSuggestedSpeedSolution();
}

void ChSystem::LCPresult_Li_into_position_cache()
{
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->ConstraintsLiFetchSuggestedPositionSolution();
		HIER_LINK_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->ConstraintsLiFetchSuggestedPositionSolution();
		HIER_OTHERPHYSICS_NEXT
	}
	this->contact_container->ConstraintsLiFetchSuggestedPositionSolution();
}

void ChSystem::LCPresult_Li_into_reactions(double mfactor)
{
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->ConstraintsFetch_react(mfactor);
		HIER_LINK_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->ConstraintsFetch_react(mfactor);
		HIER_OTHERPHYSICS_NEXT
	}
	this->contact_container->ConstraintsFetch_react(mfactor);
}


// obsolete?
void ChSystem::SetXYmode (int m_mode)
{
	modeXY = m_mode;
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		Lpointer->Set2Dmode(m_mode);
		HIER_LINK_NEXT
	}
}







//////////////////////////////////
////////
////////    COLLISION OPERATIONS
////////


int ChSystem::GetNcontacts()
{
	return this->contact_container->GetNcontacts();
}


void ChSystem::SynchronizeLastCollPositions()
{
	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		if (Bpointer->GetCollide())
			Bpointer->SynchronizeLastCollPos();
		HIER_BODY_NEXT
	}
}

 


class SystemAddCollisionPointCallback : public ChAddContactCallback
{
public: 
	ChSystem* client_system;
	virtual void ContactCallback (const collision::ChCollisionInfo& mcontactinfo, ///< pass info about contact (cannot change it)
								  ChMaterialCouple&  material 			  		///< you can modify this!
								)
	{
		if (client_system->collisionpoint_callback)
			client_system->collisionpoint_callback->ContactCallback(mcontactinfo, material);
	}	
};

double ChSystem::ComputeCollisions()
{
	double mretC= 0.0; 

	ChTimer<double> mtimer;  
	mtimer.start();

	// Update all positions of collision models	
	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		Bpointer->SyncCollisionModels();
		HIER_BODY_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->SyncCollisionModels();
		HIER_OTHERPHYSICS_NEXT
	}
 
	// Prepare the callback

	// In case there is some user callback for each added point..
	SystemAddCollisionPointCallback mpointcallback;
	if (collisionpoint_callback)
	{
		mpointcallback.client_system = this;
		this->contact_container->SetAddContactCallback(&mpointcallback);
	} else 
		this->contact_container->SetAddContactCallback(0);

	// !!! Perform the collision detection ( broadphase and narrowphase ) !!!
	
	collision_system->Run();

	// Report and store contacts and/or proximities, if there are some 
	// containers in the physic system. The default contact container
	// for ChBody and ChParticles is used always.

	collision_system->ReportContacts(this->contact_container);

	{
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		if (ChContactContainerBase* mcontactcontainer = dynamic_cast<ChContactContainerBase*>(PHpointer))
		{
			collision_system->ReportContacts(mcontactcontainer);
		}
		if (ChProximityContainerBase* mproximitycontainer = dynamic_cast<ChProximityContainerBase*>(PHpointer))
		{
			collision_system->ReportProximities(mproximitycontainer);
		}
		HIER_OTHERPHYSICS_NEXT
	}
	}

	// If some other collision engine could add further ChLinkContact into the list.. 
	if (collision_callback)
		collision_callback->PerformCustomCollision(this);

	// Count the contacts of body-body type.
	this->ncontacts = this->contact_container->GetNcontacts();

	mtimer.stop();
	this->timer_collision_broad = mtimer();

	return mretC;
}





///////////////////////////////////
////////
////////   PHYSICAL OPERATIONS
////////





//  PERFORM AN INTEGRATION STEP.  ----
//
//  Advances a single time step.
//
//  Note that time step can be modified if some variable-time stepper is used.
//
//



// internal codes for m_repeat: if FALSE (null or 0) the step won't repeat
//#define TRUE_REFINE 1
//#define TRUE_FORCED 2


int ChSystem::Integrate_Y()
{
	switch (integration_type)
	{
		case INT_ANITESCU:
			return Integrate_Y_impulse_Anitescu();
		case INT_TASORA:
			return Integrate_Y_impulse_Tasora();
		default:
			return Integrate_Y_impulse_Anitescu();
	}

	return TRUE;
}



//
//  PERFORM ANITESCU INTEGRATION STEP  -IMPULSIVE METHOD-
//
//  ...but using the differential inclusion approach, better for 
//  systems with contacts (use the Anitescu method, with stabilization)
//
 

int ChSystem::Integrate_Y_impulse_Anitescu()
{
	int ret_code = TRUE;

	ChTimer<double> mtimer_step;
	mtimer_step.start();

	events->Record(CHEVENT_TIMESTEP);

								// Executes the "forStep" script, if any
	ExecuteScriptForStep();
								// Executes the "forStep" script
								// in all controls of controlslist
	ExecuteControlsForStep();


	this->stepcount++;

	// Compute contacts and create contact constraints

	ComputeCollisions();


	Setup();	// Counts dofs, statistics, etc.


	Update();	// Update everything - and put to sleep bodies that need it

				// Re-wake the bodies that cannot sleep because they are in contact with
				// some body that is not in sleep state.
	WakeUpSleepingBodies();


	ChTimer<double> mtimer_lcp;
	mtimer_lcp.start();


	//
	// Enforce velocity/impulses constraints ....................
	//

	// reset known-term vectors
	LCPprepare_reset();

	// fill LCP known-term vectors with proper terms (forces, etc.):
	//
	// | M+dt^2*K+dt*R -Cq'|*|v_new|- | [M]*v_old + f*dt      | = |0| ,  c>=0, l>=0, l*c=0;
	// | Cq              0 | |l    |  | -C/dt +min(-C/dt,vlim)|   |c|
	//

	LCPprepare_load(true,		    // Cq,
					true,			// adds [M]*v_old to the known vector
					GetStep(),      // f*dt
					GetStep()*GetStep(), // dt^2*K  (nb only non-Schur based solvers support K matrix blocks)
					GetStep(),		// dt*R   (nb only non-Schur based solvers support R matrix blocks)
					1.0,			// M (for FEM with non-lumped masses, add their mass-matrixes)
					1.0,		    // Ct   (needed, for rheonomic motors)
					1.0/GetStep(),  // C/dt
					max_penetration_recovery_speed,	 // vlim, max penetrations recovery speed (positive for exiting)
					true);			// do above max. clamping on -C/dt

	// if warm start is used, can exploit cached multipliers from last step...
	LCPprepare_Li_from_speed_cache(); 

	// make vectors of variables and constraints, used by the following LCP solver
	LCPprepare_inject(*this->LCP_descriptor);


	// Solve the LCP problem.
	// Solution variables are new speeds 'v_new'
	GetLcpSolverSpeed()->Solve(
							*this->LCP_descriptor
							);
	mtimer_lcp.stop();
	timer_lcp = mtimer_lcp();

	// stores computed multipliers in constraint caches, maybe useful for warm starting next step 
	LCPresult_Li_into_speed_cache();

	// updates the reactions of the constraint
	LCPresult_Li_into_reactions(1.0/this->GetStep()) ; // R = l/dt  , approximately
 
	// perform an Eulero integration step (1st order stepping as pos+=v_new*dt)


	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		// EULERO INTEGRATION: pos+=v_new*dt  (do not do this, if GPU already computed it)
		if (!use_GPU)
		{
			Bpointer->VariablesQbIncrementPosition(this->GetStep());
			// Set body speed, and approximates the acceleration by differentiation.
			Bpointer->VariablesQbSetSpeed(this->GetStep());
		}
		// Now also updates all markers & forces
		Bpointer->Update(this->ChTime);
		HIER_BODY_NEXT
	}
 	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		// EULERO INTEGRATION: pos+=v_new*dt  (do not do this, if GPU already computed it)
		if (!use_GPU)
		{
			PHpointer->VariablesQbIncrementPosition(this->GetStep());
			// Set body speed, and approximates the acceleration by differentiation.
			PHpointer->VariablesQbSetSpeed(this->GetStep());
		}
		// Now also updates all markers & forces
		PHpointer->Update(this->ChTime);
		HIER_OTHERPHYSICS_NEXT
	}
 
	this->ChTime = ChTime + GetStep();

	// Executes custom processing at the end of step
	CustomEndOfStep();

	// If there are some probe objects in the probe list,
	// tell them to record their variables (ususally x-y couples)
	RecordAllProbes();

	// Time elapsed for step..
	mtimer_step.stop();
	timer_step = mtimer_step();


	return (ret_code);
}



//
//  PERFORM TASORA INTEGRATION STEP  -IMPULSIVE METHOD-
//
//  ...but using the differential inclusion approach, better for 
//  systems with contacts (use the Tasora method, with separate 
//  positional stabilization)
//


int ChSystem::Integrate_Y_impulse_Tasora()
{
	int ret_code = TRUE;

	ChTimer<double> mtimer_step;
	mtimer_step.start();

	events->Record(CHEVENT_TIMESTEP);

								// Executes the "forStep" script, if any
	ExecuteScriptForStep();
								// Executes the "forStep" script
								// in all controls of controlslist
	ExecuteControlsForStep();


	this->stepcount++;


	// Compute contacts and create contact constraints

	ComputeCollisions();


	Setup();	// Counts dofs, statistics, etc.

	Update();	// Update everything - and put to sleep bodies that need it

				// Re-wake the bodies that cannot sleep because they are in contact with
				// some body that is not in sleep state.
	WakeUpSleepingBodies();


	ChTimer<double> mtimer_lcp;
	mtimer_lcp.start();


	// 1-
	// Enforce velocity/impulses constraints ....................
	//
 
	// reset known-term vectors
	LCPprepare_reset();

	// fill LCP known-term vectors with proper terms (forces, etc.):
	//
	// | M -Cq'|*|v_new|- | [M]*v_old + f*dt    | = |0| ,  c>=0, l>=0, l*c=0;
	// | Cq  0 | |l    |  |  - Ct +min(-C/dt,0) |   |c|
	//

	LCPprepare_load(true,			// Cq
					true,			// adds [M]*v_old to the known vector
					GetStep(),      // f*dt
					GetStep()*GetStep(), // dt^2*K  (nb only non-Schur based solvers support K matrix blocks)
					GetStep(),		// dt*R   (nb only non-Schur based solvers support K matrix blocks)
					1.0,			// M (for FEM with non-lumped masses, add their mass-matrices)
					1.0,			// Ct      (needed, for rheonomic motors)
					1.0/GetStep(),  // C/dt
					0.0,			// max constr.recovery speed (positive for exiting) 
					true);			// do above max. clamping on -C/dt

	// if warm start is used, can exploit cached multipliers from last step...
	LCPprepare_Li_from_speed_cache();

	// make vectors of variables and constraints, used by the following LCP solver
	LCPprepare_inject(*this->LCP_descriptor);


	// Solve the LCP problem. 
	// Solution variables are new speeds 'v_new'
	
	GetLcpSolverSpeed()->Solve(
							*this->LCP_descriptor
							);  
		
	// stores computed multipliers in constraint caches, maybe useful for warm starting next step 
	LCPresult_Li_into_speed_cache();

	// updates the reactions of the constraint
	LCPresult_Li_into_reactions(1.0/this->GetStep()); // R = l/dt  , approximately


	// perform an Eulero integration step (1st order stepping as pos+=v_new*dt)


	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		// EULERO INTEGRATION: pos+=v_new*dt
		Bpointer->VariablesQbIncrementPosition(this->GetStep());
		// Set body speed, and approximates the acceleration by differentiation.
		Bpointer->VariablesQbSetSpeed(this->GetStep());

		// Now also updates all markers & forces
		//Bpointer->UpdateALL(this->ChTime); // not needed - will be done later anyway
		HIER_BODY_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		// EULERO INTEGRATION: pos+=v_new*dt
		PHpointer->VariablesQbIncrementPosition(this->GetStep());
		// Set body speed, and approximates the acceleration by differentiation.
		PHpointer->VariablesQbSetSpeed(this->GetStep());

		// Now also updates all markers & forces
		//PHpointer->UpdateALL(this->ChTime); // not needed - will be done later anyway
		HIER_OTHERPHYSICS_NEXT
	}

	this->ChTime = ChTime + GetStep();
 

	// 2-
	// Stabilize constraint positions ....................
	//

	// reset known-term vectors
	LCPprepare_reset();

	// Fill known-term vectors with proper terms 0 and -C :
	//
	// | M -Cq'|*|Dpos|- |0 |= |0| ,  c>=0, l>=0, l*c=0;
	// | Cq  0 | |l   |  |-C|  |c|
	//
	
	LCPprepare_load(false,  // Cq are already there.. 
					false,  // no addition of M*v in known term 
					0,		// no forces
					0,		// no K matrix
					0,		// no R matrix
					1.0,	// M (for FEM with non-lumped masses, add their mass-matrices)
					0,		// no Ct term
					1.0,	// C
					0.0,	// recovery max speed (not used)
					false);	// no clamping on -C term

	// if warm start is used, can exploit cached multipliers from last step...
	LCPprepare_Li_from_position_cache();

	// Solve the LCP problem.
	// Solution variables are 'Dpos', delta positions.

	GetLcpSolverStab()->Solve(
							*this->LCP_descriptor
							);

	// stores computed multipliers in constraint caches, maybe useful for warm starting next step 
	LCPresult_Li_into_position_cache();

	{
		HIER_BODY_INIT
		while HIER_BODY_NOSTOP
		{
			Bpointer->VariablesQbIncrementPosition(1.0); // pos+=Dpos
			// Now also updates all markers & forces
			Bpointer->Update(this->ChTime);
			HIER_BODY_NEXT
		}
		HIER_OTHERPHYSICS_INIT
		while HIER_OTHERPHYSICS_NOSTOP
		{
			PHpointer->VariablesQbIncrementPosition(1.0); // pos+=Dpos
			// Now also updates all markers & forces
			PHpointer->Update(this->ChTime);
			HIER_OTHERPHYSICS_NEXT
		} 
	}


	mtimer_lcp.stop();
	timer_lcp = mtimer_lcp();

	// Executes custom processing at the end of step
	CustomEndOfStep();

	// If there are some probe objects in the probe list,
	// tell them to record their variables (ususally x-y couples)
	RecordAllProbes();


	// Time elapsed for step..
	mtimer_step.stop();
	timer_step = mtimer_step();


	return (ret_code);
}




// **** SATISFY ALL COSTRAINT EQUATIONS WITH NEWTON
// **** ITERATION, UNTIL TOLERANCE SATISFIED, THEN UPDATE
// **** THE "Y" STATE WITH SetY (WHICH AUTOMATICALLY UPDATES
// **** ALSO AUXILIARY MATRICES).

int ChSystem::DoAssembly(int action, int mflags)
{
	Setup();
	Update();


	if (action & ASS_POSITION)		// (1)--------  POSITION
	{

		for (int m_iter = 0; m_iter < maxiter; m_iter++)
		{

			if (mflags & ASF_COLLISIONS)
			{
				// Compute new contacts and create contact constraints
				ComputeCollisions();

				Setup();	// Counts dofs, statistics, etc.
				Update();	// Update everything
			}
			

			// reset known-term vectors
			LCPprepare_reset();

			// Fill known-term vectors with proper terms 0 and -C :
			//
			// | M -Cq'|*|Dpos|- |0 |= |0| ,  c>=0, l>=0, l*c=0;
			// | Cq  0 | |l   |  |-C|  |c|
			//

			LCPprepare_load(true,  // Cq 
					false,	// no addition of M*v in known term 
					0,		// no forces
					0,		// no K matrix
					0,		// no R matrix
					1.0,	// M (for FEM with non-lumped masses, add their mass-matrices)
					0,		// no Ct term
					1.0,	// C
					0.0,	// 
					false);	// no clamping on -C/dt
			
			// make the vectors of pointers to constraint and variables, for LCP solver
			LCPprepare_inject(*this->LCP_descriptor);

			// exit Newton loop if reached tolerance..
			double max_res, max_LCPerr;
			this->LCP_descriptor->ComputeFeasabilityViolation(max_res, max_LCPerr);
			if (max_res <= this->tol)
			{
				//reached_tolerance = TRUE;
				break;		// stop Newton when reached C tolerance |||||||| :-)
			}


				// Solve the LCP problem.
				// Solution variables are 'Dpos', delta positions.
				// Note: use settings of the 'speed' lcp solver (i.e. use max number
				// of iterations as you would use for the speed probl., if iterative solver)
			GetLcpSolverSpeed()->Solve(
									*this->LCP_descriptor
									);

			// Move bodies to updated position
			HIER_BODY_INIT
			while HIER_BODY_NOSTOP
			{
				Bpointer->VariablesQbIncrementPosition(1.0); // pos+=Dpos
				Bpointer->Update(this->ChTime);
				HIER_BODY_NEXT
			}
			HIER_OTHERPHYSICS_INIT
			while HIER_OTHERPHYSICS_NOSTOP
			{
				PHpointer->VariablesQbIncrementPosition(1.0); // pos+=Dpos
				PHpointer->Update(this->ChTime);
				HIER_OTHERPHYSICS_NEXT
			}

			Update(); // Update everything

		} // end loop Newton iterations

	}

	if ((action & ASS_SPEED)||(action & ASS_ACCEL))			// 2) -------- SPEEDS and ACCELERATIONS
	{
		double foo_dt = 1e-7;

		// reset known-term vectors
		LCPprepare_reset();

		// fill LCP known-term vectors with proper terms
		//
		// | M -Cq'|*|v_new|- | [M]*v_old +f*dt | = |0| ,  c>=0, l>=0, l*c=0;
		// | Cq  0 | |l    |  |  - Ct           |   |c|
		//
		LCPprepare_load(false,  // Cq are already there.. 
					true,	    // adds [M]*v_old to the known vector
					foo_dt,		// f*dt
					foo_dt*foo_dt,// dt^2*K  (nb only non-Schur based solvers support K matrix blocks)
					foo_dt,		// dt*R   (nb only non-Schur based solvers support K matrix blocks)
					1.0,			// M (for FEM with non-lumped masses, add their mass-matrices)
					1.0,		// Ct term
					0,			// no C term
					0.0,		// 
					false);		// no clamping on -C/dt

		
		// make the vectors of pointers to constraint and variables, for LCP solver
		LCPprepare_inject(*this->LCP_descriptor);

		// Solve the LCP problem with iterative Gauss-Seidel solver. Solution
		// variables are new speeds 'v_new'
		GetLcpSolverSpeed()->Solve(
								*this->LCP_descriptor
								);	


		HIER_BODY_INIT
		while HIER_BODY_NOSTOP
		{
			// Set body speed -  and approximates the acceleration by BDF, with step foo_dt
			Bpointer->VariablesQbSetSpeed(foo_dt);
			// Now also updates all markers & forces
			Bpointer->Update(this->ChTime);
			HIER_BODY_NEXT
		}
		HIER_OTHERPHYSICS_INIT
		while HIER_OTHERPHYSICS_NOSTOP
		{
			// Set speed -  and approximates the acceleration by BDF, with step foo_dt
			PHpointer->VariablesQbSetSpeed(foo_dt);
			// Now also updates all markers & forces
			PHpointer->Update(this->ChTime);
			HIER_OTHERPHYSICS_NEXT
		}

	}

	return 0;
}





// **** PERFORM THE LINEAR STATIC ANALYSIS

int ChSystem::DoStaticLinear()
{
	Setup();
	Update();

		// reset known-term vectors
	LCPprepare_reset();

	// Fill known-term vectors with proper terms 0 and -C :
	//
	// | K   -Cq'|*|Dpos|- |f |= |0| ,  c>=0, l>=0, l*c=0;
	// | Cq    0 | |l   |  |C |  |c|
	//

	LCPprepare_load(true,  // Cq 
			false,	// no addition of [M]*v_old to the known vector
			1.0,	// f  forces
			1.0,	// K  matrix
			0,		// no R matrix
			0,		// no M matrix (for FEM with non-lumped masses, do not add their mass-matrices)
			0,		// no Ct term
			1.0,	// C constraint gap violation, if any
			0.0,	// 
			false);	// no clamping on -C/dt
	
	// make the vectors of pointers to constraint and variables, for LCP solver
	LCPprepare_inject(*this->LCP_descriptor);

//***DEBUG***
/*
try
{
	chrono::ChSparseMatrix mdMK;
	chrono::ChSparseMatrix mdCq;
	chrono::ChSparseMatrix mdE;
	chrono::ChMatrixDynamic<double> mdf;
	chrono::ChMatrixDynamic<double> mdb;
	chrono::ChMatrixDynamic<double> mdfric;
	this->LCP_descriptor->ConvertToMatrixForm(&mdCq, &mdMK, &mdE, &mdf, &mdb, &mdfric);
	chrono::ChStreamOutAsciiFile file_MK("dump_MK.dat");
	mdMK.StreamOUTsparseMatlabFormat(file_MK);
	chrono::ChStreamOutAsciiFile file_Cq("dump_Cq.dat");
	mdCq.StreamOUTsparseMatlabFormat(file_Cq);
	chrono::ChStreamOutAsciiFile file_E("dump_E.dat");
	mdE.StreamOUTsparseMatlabFormat(file_E);
	chrono::ChStreamOutAsciiFile file_f("dump_f.dat");
	mdf.StreamOUTdenseMatlabFormat(file_f);
	chrono::ChStreamOutAsciiFile file_b("dump_b.dat");
	mdb.StreamOUTdenseMatlabFormat(file_b);
} 
catch(chrono::ChException myexc)
{
	chrono::GetLog() << myexc.what();
}
*/


		// Solve the LCP problem.
		// Solution variables are 'Dpos', delta positions.
		// Note: use settings of the 'speed' lcp solver (i.e. use max number
		// of iterations as you would use for the speed probl., if iterative solver)
	GetLcpSolverSpeed()->Solve(
							*this->LCP_descriptor
							);	

//***DEBUG***
/*
// **CHECK*** optional check for correctness in result
chrono::ChMatrixDynamic<double> md;
GetLcpSystemDescriptor()->BuildDiVector(md);			// d={f;-b}

chrono::ChMatrixDynamic<double> mx;
GetLcpSystemDescriptor()->FromUnknownsToVector(mx);		// x ={q,-l}
chrono::ChStreamOutAsciiFile file_x("dump_x.dat");
mx.StreamOUTdenseMatlabFormat(file_x);

chrono::ChMatrixDynamic<double> mZx;
GetLcpSystemDescriptor()->SystemProduct(mZx, &mx);		// Zx = Z*x

GetLog() << "CHECK: norm of solver residual: ||Z*x-d|| -------------------\n";
GetLog() << (mZx - md).NormInf() << "\n";
*/

	// Updates the reactions of the constraint, getting them from solver data
	LCPresult_Li_into_reactions(1.0) ; 

	// Move bodies to updated position
	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
		Bpointer->VariablesQbIncrementPosition(1.0); // pos+=Dpos
		Bpointer->Update(this->ChTime);
		HIER_BODY_NEXT
	}
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		PHpointer->VariablesQbIncrementPosition(1.0); // pos+=Dpos
		PHpointer->Update(this->ChTime);
		HIER_OTHERPHYSICS_NEXT
	}

	Update(); // Update everything

	return 0;
}




// **** PERFORM THE STATIC ANALYSIS, FINDING THE STATIC
// **** EQUILIBRIUM OF THE SYSTEM, WITH ITERATIVE SOLUTION

int ChSystem::DoStaticRelaxing ()
{
	int err = 0;
	int reached_tolerance = FALSE;

	ResetErrors();

	if (ncoords > 0)
	{
		if (ndof >= 0)
		{
			for (int m_iter = 0; m_iter < STATIC_MAX_STEPS; m_iter++)
			{
				HIER_BODY_INIT
				while HIER_BODY_NOSTOP
				{
					// Set no body speed and no body accel.
					Bpointer->SetNoSpeedNoAcceleration();
					HIER_BODY_NEXT
				}
				HIER_OTHERPHYSICS_INIT
				while HIER_OTHERPHYSICS_NOSTOP
				{
					PHpointer->SetNoSpeedNoAcceleration();
					HIER_OTHERPHYSICS_NEXT
				}

				double m_undotime = this->GetChTime();
				DoFrameDynamics(m_undotime + (this->GetStep()*1.8)*( ((double)STATIC_MAX_STEPS-(double)m_iter))/(double)STATIC_MAX_STEPS );
				this->SetChTime(m_undotime);
			}


			HIER_BODY_INIT
			while HIER_BODY_NOSTOP
			{
				// Set no body speed and no body accel.
				Bpointer->SetNoSpeedNoAcceleration();
				HIER_BODY_NEXT
			}
			HIER_OTHERPHYSICS_INIT
			while HIER_OTHERPHYSICS_NOSTOP
			{
				PHpointer->SetNoSpeedNoAcceleration();
				HIER_OTHERPHYSICS_NEXT
			}
		}
	}

	if (err)
	{
		last_err = TRUE;
		GetLog() << "WARNING: some costraints may be redundant, but couldn't be eliminated \n";
	}
	return last_err;
}


// **** ---    THE KINEMATIC SIMULATION  ---
// **** PERFORM IK (INVERSE KINEMATICS) UNTIL THE END_TIME IS
// **** REACHED, STARTING FROM THE CURRENT TIME.


int ChSystem::DoEntireKinematics()
{
	Setup();

	ResetErrors();

	DoAssembly (ASS_POSITION|ASS_SPEED|ASS_ACCEL);
						// first check if there are redundant links (at least one NR cycle
						// even if the structure is already assembled)

	while (ChTime < end_time)
	{
		DoAssembly (ASS_POSITION|ASS_SPEED|ASS_ACCEL);		// >>> Newton-Raphson iteration, closing constraints

		if (last_err) return FALSE;

		ChTime += step;		// >>> Update the time and repeat.
	}

	return TRUE;
}




// **** ---   THE DYNAMICAL SIMULATION   ---
// **** PERFORM EXPLICIT OR IMPLICIT INTEGRATION TO GET
// **** THE DYNAMICAL SIMULATION OF THE SYSTEM, UNTIL THE
// **** END_TIME IS REACHED.


int ChSystem::DoEntireDynamics()
{

	Setup();

	ResetErrors();

			// the system may have wrong layout, or too large
			// clearances in costraints, so it is better to
			// check for costraint violation each time the integration starts
	DoAssembly (ASS_POSITION|ASS_SPEED|ASS_ACCEL);

			// Perform the integration steps until the end
			// time is reached.
			// All the updating (of Y, Y_dt and time) is done
			// automatically by Integrate()

	while (ChTime < end_time)
	{
		if (!Integrate_Y ()) break;	// >>> 1- single integration step,
									//        updating Y, from t to t+dt.
		if (last_err) return FALSE;
	}

	if (last_err) return FALSE;
	return TRUE;
}


int ChSystem::DoStepDynamics (double m_step)
{
	this->step=m_step;
	return Integrate_Y ();
}



// Perform the dynamical integration, from current ChTime to
// the specified m_endtime, and terminating the integration exactly
// on the m_endtime. Therefore, the step of integration may get a
// little increment/decrement to have the last step ending in m_endtime.
// Note that this function can be used in iterations to provide results in
// a evenly spaced frames of time, even if the steps are changing.
// Also note that if the time step is higher than the time increment
// requested to reach m_endtime, the step is lowered.

int ChSystem::DoFrameDynamics (double m_endtime)
{
	double frame_step;
	double old_step;
	double left_time;
	int restore_oldstep = FALSE;
	int counter = 0;
	double fixed_step_undo;


	ResetErrors();

	frame_step = (m_endtime - ChTime);
	fixed_step_undo = step;

	while (ChTime < m_endtime)
	{
		restore_oldstep = FALSE;
		counter++;

		left_time = m_endtime - ChTime;

		if (left_time < 0.0000000000001) break;		// - no integration if backward or null frame step.

		if (left_time < (1.3* step))			// - step changed if too little frame step
		{
			old_step = step;
			step = left_time;
			restore_oldstep = TRUE;
		}


		if (!Integrate_Y ()) break;	// ***  Single integration step,
									// ***  updating Y, from t to t+dt.
									// ***  This also changes local ChTime, and may change step

		if (this->adaption == STEP_FIXED) // this because collision detection may rewind time a bit, even if step_fixed
				step = fixed_step_undo;

		if (last_err) break;
	}

	if (restore_oldstep)
		step = old_step; // if timestep was changed to meet the end of frametime, restore pre-last (even for time-varying schemes)
	if (this->adaption == STEP_FIXED)
		step = fixed_step_undo;	// anyway, restore original step if no adaption

	if (last_err) return FALSE;
	return TRUE;
}


// Performs the dynamical simulation, but using "frame integration"
// iteratively. The results are provided only at each frame (evenly
// spaced by "frame_step") rather than at each "step" (steps can be much
// more than frames, and they may be automatically changed by integrator).
// Moreover, the integration results shouldn't be dependent by the
// "frame_step" value (steps are performed anyway, like in normal "DoEntireDynamics"
// command).

int ChSystem::DoEntireUniformDynamics(double frame_step)
{

			// the initial system may have wrong layout, or too large
			// clearances in costraints.
	Setup();
	DoAssembly (ASS_POSITION|ASS_SPEED|ASS_ACCEL);

	while (ChTime < end_time)
	{
		double goto_time = (ChTime + frame_step);
		if (!DoFrameDynamics(goto_time)) return FALSE;		// ###### Perform "frame integration
	}

	return TRUE;
}


// Like DoFrameDynamics, but performs kinematics instead of dinamics

int ChSystem::DoFrameKinematics (double m_endtime)
{
	double frame_step;
	double old_step;
	double left_time;
	int restore_oldstep;
	int counter = 0;


	ResetErrors();

	frame_step = (m_endtime - ChTime);

	double fixed_step_undo = step;

	while (ChTime < m_endtime)
	{
		restore_oldstep = FALSE;
		counter++;

		left_time = m_endtime - ChTime;

		if (left_time < 0.000000001) break;		// - no kinematics for backward

		if (left_time < (1.3* step))			// - step changed if too little frame step
			{
				old_step = step;
				step = left_time;
				restore_oldstep = TRUE;
			}


		DoAssembly(ASS_POSITION|ASS_SPEED|ASS_ACCEL);		// ***  Newton Raphson kinematic equations solver

		if (last_err) return FALSE;

		ChTime += step;

		if (restore_oldstep) step = old_step; // if timestep was changed to meet the end of frametime
	}

	if (this->adaption == STEP_FIXED)
		step = fixed_step_undo;	// anyway, restore original step if no adaption

	return TRUE;
}



int ChSystem::DoStepKinematics (double m_step)
{
	ResetErrors();
	
	ChTime += m_step;

	Update(); 

	DoAssembly(ASS_POSITION|ASS_SPEED|ASS_ACCEL);		// ***  Newton Raphson kinematic equations solver

	if (last_err) return FALSE;

	return TRUE;
}


//
// Full assembly -computes also forces-
//

int ChSystem::DoFullAssembly()
{
	ResetErrors();

	DoAssembly(ASS_POSITION|ASS_SPEED|ASS_ACCEL);

	return last_err;
}








////////
////////  STREAMING - FILE HANDLING
////////


void ChSystem::StreamOUT(ChStreamOutBinary& mstream)
{
			// class version number
	mstream.VersionWrite(5);

		// serialize parent class too
	ChObj::StreamOUT(mstream);

		// stream out all member data
	mstream << GetEndTime();
	mstream << GetStep();
	mstream << GetStepMin();
	mstream << GetStepMax();
	mstream << GetTol();
	mstream << GetNormType();
	mstream << GetMaxiter();
	mstream << (int)GetIntegrationType();
	mstream << GetOrder();
	mstream << GetMultisteps();
	mstream << GetAdaption();
	mstream << 0; //GetBaumgarteStabilize();
	mstream << GetDynaclose();
	mstream << GetDynatol();
	mstream << GetPredict();
	mstream << GetPredorder();
	mstream << GetStifftol();
	mstream << G_acc;
	mstream << GetXYmode();
	mstream << GetNsClosePos();
	mstream << GetNsCloseSpeed();
	mstream << GetMonolattol();
	mstream << GetIntegrtol();
	mstream << GetAutoAssembly();
	mstream << GetScriptForStartFile();
	mstream << GetScriptForUpdateFile();
	mstream << GetScriptForStepFile();
	mstream << GetScriptFor3DStepFile();
	mstream << GetMaxStepsCollide();
	mstream << GetMinBounceSpeed();
	// v2
	mstream << iterLCPmaxIters;
	mstream << iterLCPmaxItersStab;
	mstream << simplexLCPmaxSteps;
	mstream << (int)GetLcpSolverType();
	// v3,v4
	mstream << (int)0;//GetFrictionProjection();
	// v5
	mstream << parallel_thread_number;
	mstream << max_penetration_recovery_speed;
	mstream << use_GPU;
	// v6   
	mstream << use_sleeping;
}

void ChSystem::StreamIN(ChStreamInBinary& mstream)
{
		// class version number
	int version = mstream.VersionRead();

		// deserialize parent class too
	ChObj::StreamIN(mstream);

		// stream in all member data
	 double mdouble;
	 int mint;
	 Vector mvector;
	 char buffer[250];
	mstream >> mdouble;		SetEndTime(mdouble);
	mstream >> mdouble;		SetStep(mdouble);
	mstream >> mdouble;		SetStepMin(mdouble);
	mstream >> mdouble;		SetStepMax(mdouble);
	mstream >> mdouble;		SetTol(mdouble);
	mstream >> mint;		SetNormType (mint);
	mstream >> mint;		SetMaxiter(mint);
	mstream >> mint;		SetIntegrationType ((eCh_integrationType)mint);
	mstream >> mint;		SetOrder(mint);
	mstream >> mint;		SetMultisteps(mint);
	mstream >> mint;		SetAdaption (mint);
	mstream >> mint;		//SetBaumgarteStabilize(mint);
	mstream >> mint;		SetDynaclose(mint);
	mstream >> mdouble;		SetDynatol(mdouble);
	mstream >> mint;		SetPredict(mint);
	mstream >> mint;		SetPredorder(mint);
	mstream >> mdouble;		SetStifftol(mdouble);
	mstream >> mvector;		Set_G_acc(mvector);
	mstream >> mint;		SetXYmode(mint);
	mstream >> mint;		SetNsClosePos(mint);
	mstream >> mint;		SetNsCloseSpeed(mint);
	mstream >> mdouble;		SetMonolattol(mdouble);
	mstream >> mdouble;		SetIntegrtol(mdouble);
	mstream >> mint;		SetAutoAssembly(mint);
	mstream >> buffer;		SetScriptForStartFile(buffer);
	mstream >> buffer;		SetScriptForUpdateFile(buffer);
	mstream >> buffer;		SetScriptForStepFile(buffer);
	mstream >> buffer;		SetScriptFor3DStepFile(buffer);
	mstream >> mint;		SetMaxStepsCollide(mint);
	mstream >> mdouble;		SetMinBounceSpeed(mdouble);

	if (version>=2)
	{
		mstream >> iterLCPmaxIters;
		mstream >> iterLCPmaxItersStab;
		mstream >> simplexLCPmaxSteps;
		mstream >> mint;	SetLcpSolverType((eCh_lcpSolver) mint);
	}
	if (version>=3)
	{
		mstream >> mint;	//SetFrictionProjection((eCh_frictionProjection) mint);
		if (version==3) {}; //SetFrictionProjection(FRI_CONEORTHO); // for v3, ortho proj anyway
	}
	if (version>=5)
	{
		mstream >> parallel_thread_number;
		mstream >> max_penetration_recovery_speed;
		mstream >> use_GPU;
	}
	if (version>=6)
	{
		mstream >> use_sleeping;
	}
}

void ChSystem::StreamOUT(ChStreamOutAscii& mstream)
{
	//***TO DO***
}

#define CH_CHUNK_END   1234
#define CH_CHUNK_START 4321

int ChSystem::StreamINall  (ChStreamInBinary& m_file)
{
	int mchunk = 0;
	ChBody* newbody= NULL;
	ChLink* newlink= NULL;
	ChPhysicsItem* newitem= NULL;

	// class version number
	int version = m_file.VersionRead();

	// 0) reset system to have no sub object child
	this->Clear();

	// 1) read system class data...
	m_file >> *this;

	// 2a) read how many bodies
	int mnbodies = 0;
	m_file >> mnbodies;

	// 2b) read  bodies
	for (int i= 0; i<mnbodies; i++)
	{
		ChSharedPtr<ChBody> newbody(new ChBody);
		this->AddBody(newbody);

		if (!newbody->StreamINall(m_file)) 
			throw ChException("Cannot read body data");
	}

	// 3a) read how many links
	int mnlinks = 0;
	m_file >> mnlinks;

	// 3b) read  links
	for (int j= 0; j<mnlinks; j++)
	{
			// read the link, using a special downcasting function Link_BinRead_Create which creates the
			// proper inherited object, depending on its class inheritance from base Link*

		m_file.AbstractReadCreate(&newlink);
		if (!newlink) throw ChException("Cannot read link data");

		ChSharedPtr<ChLink> shlink(newlink);
		this->AddLink(shlink);
	}

	// 3c) Rebuild link pointers to markers
	this->Reference_LM_byID();


	// 4a) read how many other physics item
	int mnitems = 0;
	m_file >> mnitems;

	// 4b) read physics items
	for (int j= 0; j<mnitems; j++)
	{
			// read the item, using a special downcasting function which creates the
			// proper inherited object, depending on its class inheritance from base ChPhysicsItem*

		m_file.AbstractReadCreate(&newitem);
		if (!newitem) throw ChException("Cannot read ChPhysicsItem data.");

		ChSharedPtr<ChPhysicsItem> shitem(newitem);
		this->AddOtherPhysicsItem(shitem);
	}


	this->Setup();

	return 1;
}

int ChSystem::StreamOUTall  (ChStreamOutBinary& m_file)
{
	// class version number
	m_file.VersionWrite(2);

	// 1) write system class data...
	m_file << *this;

	// 2a) write how many bodies
	m_file << (int)bodylist.size();//this->ListCount((ChObj**)&bodylist); ***SHAREDBODY***

	// 2b) write  bodies
	HIER_BODY_INIT
	while HIER_BODY_NOSTOP
	{
			// write the body + child markers + forces
		if (!Bpointer->StreamOUTall(m_file)) return 0;
		HIER_BODY_NEXT
	}

	// 3a) write how many links
	m_file << (int)linklist.size(); 

	// 3b) write links links
	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
			// Writethe link, using a special downcasting function Link_BinSave which saves also the
			// inheritance info, depending on link class inheritance from base Link*
		m_file.AbstractWrite(Lpointer);

		HIER_LINK_NEXT
	}

	// 4a) write how many other physics items
	m_file << (int)otherphysicslist.size(); 

	// 4b) write other physics item links
	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
			// Write the item, using a special downcasting function which saves also the
			// inheritance info, depending on class inheritance from base ChPhysicsItem*
		m_file.AbstractWrite(PHpointer);

		HIER_OTHERPHYSICS_NEXT
	}

	m_file << (int)CH_CHUNK_END;

	return 1;
}

void ChSystem::ShowHierarchy(ChStreamOutAscii& m_file)
{
	m_file << "\n   List of the " << (int)Get_bodylist()->size() << " added rigid bodies: \n";
 
	std::vector<ChBody*>::iterator ibody = Get_bodylist()->begin();
	while (ibody != Get_bodylist()->end())
	{
		GetLog() << "     BODY:       " << (*ibody)->GetName() << "\n";

		std::vector<ChMarker*>::iterator imarker = (*ibody)->GetMarkerList()->begin();
		while (imarker != (*ibody)->GetMarkerList()->end())
		{
			GetLog() << "        MARKER:   " << (*imarker)->GetName() << "\n";
			imarker++;
		}

		std::vector<ChForce*>::iterator iforce = (*ibody)->GetForceList()->begin();
		while (iforce != (*ibody)->GetForceList()->end())
		{
			GetLog() << "        FORCE:   " << (*iforce)->GetName() << "\n";
			iforce++;
		}

		ibody++;
	}

	m_file << "\n   List of the " << (int)Get_linklist()->size() << " added links: \n";

	HIER_LINK_INIT
	while HIER_LINK_NOSTOP
	{
		GetLog() << "     LINK:       " << Lpointer->GetName() << "\n";
		if (ChLinkMarkers* malink = ChDynamicCast(ChLinkMarkers,Lpointer))
		{
			if(malink->GetMarker1())
			GetLog() << "        marker1:     " << malink->GetMarker1()->GetName() << "\n";
			if(malink->GetMarker2())
			GetLog() << "        marker2:     " << malink->GetMarker2()->GetName() << "\n";
		}
		HIER_LINK_NEXT
	}

	
	m_file << "\n   List of other " << (int)otherphysicslist.size() << " added physic items: \n";

	HIER_OTHERPHYSICS_INIT
	while HIER_OTHERPHYSICS_NOSTOP
	{
		GetLog() << "     PHYSIC ITEM :       " << PHpointer->GetName() << "\n";
		HIER_OTHERPHYSICS_NEXT
	}

	m_file << "\n\nFlat ChPhysicalItem list (class name - object name):----- \n\n";

	IteratorAllPhysics mphiter(this);
	while(mphiter.HasItem())
	{
		GetLog() << "  " <<   mphiter->GetRTTI()->GetName() << "  -  " <<   mphiter->GetName() << "\n";
		++mphiter;
	}
	m_file << "\n\n";
}

int ChSystem::FileProcessChR (ChStreamInBinary& m_file)
{
	int mchunk = 0;

	m_file >> mchunk;
	if (mchunk != CH_CHUNK_START) 
		throw ChException("Not a ChR data file.");

	this->StreamINall(m_file);

	m_file >> mchunk;
	if (mchunk != CH_CHUNK_END)
		throw ChException("The end of ChR data file is badly formatted.");

	return 1;
}

int ChSystem::FileWriteChR (ChStreamOutBinary& m_file)
{
	m_file << (int)CH_CHUNK_START;

	this->StreamOUTall(m_file);

	m_file << (int)CH_CHUNK_END;

	return 1;
}

	// process a scripting instruction file
int ChSystem::FileProcessJS (char* m_file)
{
	if (!this->scriptEngine) return 0;

	ChScript* mscript = this->scriptEngine->CreateScript();
	if (!this->scriptEngine->ExecuteScript(*mscript)) return 0;
	delete mscript;
	return 1;
}






//////////////////////////////////////////////////////////////////


ChSystem::IteratorBodies& ChSystem::IteratorBodies::operator=(const IteratorBodies& other)
{
	node_ = other.node_; 
	return(*this);
}
bool ChSystem::IteratorBodies::operator==(const IteratorBodies& other)
{
	return(node_ == other.node_);
}
bool ChSystem::IteratorBodies::operator!=(const IteratorBodies& other)
{
	return(node_ != other.node_);
}
ChSystem::IteratorBodies& ChSystem::IteratorBodies::operator++()
{
	node_++;
	return(*this);
}
ChSharedPtr<ChBody> ChSystem::IteratorBodies::operator*()
{
	(*node_)->AddRef(); // needed because ...
	return (ChSharedPtr<ChBody>((*node_)));  // .. here I am not getting a new() data, but a reference to something created elsewhere
}
ChSystem::IteratorBodies ChSystem::IterBeginBodies() {return (IteratorBodies(this->bodylist.begin()));};
ChSystem::IteratorBodies ChSystem::IterEndBodies() {return (IteratorBodies(this->bodylist.end()));};



//////////////////////////////////////////////////////////////////


ChSystem::IteratorLinks& ChSystem::IteratorLinks::operator=(const IteratorLinks& other)
{
	node_ = other.node_;
	return(*this);
}
bool ChSystem::IteratorLinks::operator==(const IteratorLinks& other)
{
	return(node_ == other.node_);
}
bool ChSystem::IteratorLinks::operator!=(const IteratorLinks& other)
{
	return(node_ != other.node_);
}
ChSystem::IteratorLinks& ChSystem::IteratorLinks::operator++()
{
	node_++;
	return(*this);
}
ChSharedPtr<ChLink> ChSystem::IteratorLinks::operator*()
{
	(*node_)->AddRef(); // needed because ...
	return (ChSharedPtr<ChLink>((*node_)));  // .. here I am not getting a new() data, but a reference to something created elsewhere
}
ChSystem::IteratorLinks ChSystem::IterBeginLinks() {return (IteratorLinks(this->linklist.begin()));};
ChSystem::IteratorLinks ChSystem::IterEndLinks() {return (IteratorLinks(this->linklist.end()));};



//////////////////////////////////////////////////////////////////


ChSystem::IteratorOtherPhysicsItems& ChSystem::IteratorOtherPhysicsItems::operator=(const IteratorOtherPhysicsItems& other)
{
	node_ = other.node_;
	return(*this);
}
bool ChSystem::IteratorOtherPhysicsItems::operator==(const IteratorOtherPhysicsItems& other)
{
	return(node_ == other.node_);
}
bool ChSystem::IteratorOtherPhysicsItems::operator!=(const IteratorOtherPhysicsItems& other)
{
	return(node_ != other.node_);
}
ChSystem::IteratorOtherPhysicsItems& ChSystem::IteratorOtherPhysicsItems::operator++()
{
	node_++; 
	return(*this);
}
ChSharedPtr<ChPhysicsItem> ChSystem::IteratorOtherPhysicsItems::operator*()
{
	(*node_)->AddRef(); // needed because ...
	return (ChSharedPtr<ChPhysicsItem>((*node_)));  // .. here I am not getting a new() data, but a reference to something created elsewhere
}
ChSystem::IteratorOtherPhysicsItems ChSystem::IterBeginOtherPhysicsItems() {return (IteratorOtherPhysicsItems(this->otherphysicslist.begin()));};
ChSystem::IteratorOtherPhysicsItems ChSystem::IterEndOtherPhysicsItems() {return (IteratorOtherPhysicsItems(this->otherphysicslist.end()));};



//////////////////////////////////////////////////////////////////


ChSystem::IteratorPhysicsItems::IteratorPhysicsItems(ChSystem* msys) 
{
	this->msystem = msys;
	//RewindToBegin();
	node_body   = msystem->Get_bodylist()->begin();
	node_link	  = msystem->Get_linklist()->begin();
	node_otherphysics	 = msystem->Get_otherphysicslist()->begin();
	stage = 0;
    mptr = 0;
	this->operator++(); // initialize with 1st available item
}
ChSystem::IteratorPhysicsItems::IteratorPhysicsItems()
{
	this->msystem = 0;
	this->mptr    = 0;
	this->stage   = 9999;  
}

//~ChSystem::IteratorPhysicsItems::IteratorPhysicsItems() {}
/*
void ChSystem::IteratorPhysicsItems::RewindToBegin()
{
  node_body   = msystem->Get_bodylist()->begin();
  node_link	  = msystem->Get_linklist()->begin();
  node_otherphysics	 = msystem->Get_otherphysicslist()->begin();
  stage = 0;
  mptr = 0;
  this->operator++(); // initialize with 1st available item
}

bool ChSystem::IteratorPhysicsItems::ReachedEnd()
{
  if (stage == 9999)
	  return true;
  return false;
}
*/
bool ChSystem::IteratorPhysicsItems::HasItem()
{
  if (mptr)
	  return true;
  return false;
}

ChSystem::IteratorPhysicsItems& ChSystem::IteratorPhysicsItems::operator=(const ChSystem::IteratorPhysicsItems& other)
{
  msystem			= other.msystem;
  node_body			= other.node_body;
  node_link			= other.node_link;
  node_otherphysics	= other.node_otherphysics;
  stage				= other.stage;
  mptr				= other.mptr;
  return(*this);
}

bool ChSystem::IteratorPhysicsItems::operator==(const ChSystem::IteratorPhysicsItems& other)
{
	return ( (mptr == other.mptr) && 
		     (stage == other.stage) &&
			 (msystem == other.msystem) ); //...***TO CHECK***
}

bool ChSystem::IteratorPhysicsItems::operator!=(const ChSystem::IteratorPhysicsItems& other)
{
	return!( this->operator==(other)); //...***TO CHECK***
}

ChSystem::IteratorPhysicsItems& ChSystem::IteratorPhysicsItems::operator++()
{
  switch (stage)
  {
	case 1:
	{
		node_body++; // try next body
		if (node_body != msystem->Get_bodylist()->end())
		{
			mptr = (*node_body); 
			return (*this);
		} 
		break;
	}
	case 2:
	{
		node_link++; // try next link
		if (node_link != msystem->Get_linklist()->end())
		{
			mptr = (*node_link); 
			return (*this);
		}
		break;
	}
	case 3:
	{
		node_otherphysics++; // try next otherphysics
		if (node_otherphysics != msystem->Get_otherphysicslist()->end())
		{
			mptr = (*node_otherphysics); 
			return (*this);
		}
		break;
	}
	default:
		break;
  }
  // Something went wrong, some list was at the end, so jump to beginning of next list
  do
  {
	  switch(stage)
	  {
	  case 0:
		  {
				  stage = 1;
				  if (node_body != msystem->Get_bodylist()->end())
				  {
					  mptr = (*node_body); 
					  return (*this);
				  } 
				  break;
		  }
	  case 1:
		  {
				  stage = 2;
				  if (node_link != msystem->Get_linklist()->end())
				  {
					  mptr = (*node_link); 
					  return (*this);
				  } 
				  break;
		  }
	  case 2:
		  {
				  stage = 3;
				  if (node_otherphysics != msystem->Get_otherphysicslist()->end())
				  {
					  mptr = (*node_otherphysics); 
					  return (*this);
				  } 
				  break;
		  }
	  case 3:
		  {
				  stage = 4;
				  mptr = msystem->GetContactContainer(); 
				  return (*this);
		  }
	  case 4:
		  {
				  stage = 9999;
				  mptr = 0; 
				  return (*this);
		  }
	  } // end cases
  } while (true);

 return(*this);
}

ChSharedPtr<ChPhysicsItem> ChSystem::IteratorPhysicsItems::operator*()
{
	mptr->AddRef(); // needed because ...
	return (ChSharedPtr<ChPhysicsItem>(mptr));  // .. here I am not getting a new() data, but a reference to something created elsewhere
}
ChSystem::IteratorPhysicsItems ChSystem::IterBeginPhysicsItems() {return (IteratorPhysicsItems(this));};
ChSystem::IteratorPhysicsItems ChSystem::IterEndPhysicsItems() {return (IteratorPhysicsItems());};




} // END_OF_NAMESPACE____

/////////////////////////////////////// eof

