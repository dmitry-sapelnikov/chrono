//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//
// File authors: Andrea Favali, Alessandro Tasora

#ifndef CHMESH_H
#define CHMESH_H


#include <stdlib.h>
#include <math.h>

#include "physics/ChPhysicsItem.h"
#include "physics/ChContinuumMaterial.h"
#include "ChNodeFEMbase.h"
#include "ChElementBase.h"

namespace chrono 
{
namespace fem
{



/// Class which defines a mesh of finite elements of class ChFelem,
/// between nodes of class  ChFnode. 

class ChApiFem ChMesh : public ChIndexedNodes
{
private:

	std::vector<ChNodeFEMbase*>	 vnodes;	//  nodes
	std::vector<ChElementBase*>	 velements;	//  elements

	unsigned int n_dofs; // total degrees of freedom


public:

	ChMesh() { n_dofs = 0;};
	~ChMesh() {};

	void AddNode (ChNodeFEMbase& m_node);
	void AddElement (ChElementBase& m_elem);
	void ClearNodes ();
	void ClearElements ();
	
				/// Access the N-th node 
	virtual ChNodeBase* GetNode(unsigned int n) {return vnodes[n];};
				/// Access the N-th element 
	virtual ChElementBase* GetElement(unsigned int n) {return velements[n];};

	unsigned int GetNnodes () {return vnodes.size();}
	unsigned int GetNelements () {return velements.size();}
	virtual  int GetDOF () {return n_dofs;}

				/// - Computes the total number of degrees of freedom
				/// - Precompute auxiliary data, such as (local) stiffness matrices Kl, if any, for each element.
	void SetupInitial ();				

				/// Set reference position of nodes as current position, for all nodes.
	void Relax ();

				/// Update time dependent data, for all elements. 
				/// Updates all [A] coord.systems for all (corotational) elements.
	void Update(double m_time);
			


			//
			// LCP SYSTEM FUNCTIONS        for interfacing all elements with LCP solver
			//

				/// Tell to a system descriptor that there are items of type
				/// ChLcpKstiffness in this object (for further passing it to a LCP solver)
				/// Basically does nothing, but maybe that inherited classes may specialize this.
	virtual void InjectKRMmatrices(ChLcpSystemDescriptor& mdescriptor);

				/// Adds the current stiffness K and damping R and mass M matrices in encapsulated
				/// ChLcpKstiffness item(s), if any. The K, R, M matrices are added with scaling 
				/// values Kfactor, Rfactor, Mfactor.  
	virtual void KRMmatricesLoad(double Kfactor, double Rfactor, double Mfactor);


				/// Sets the 'fb' part (the known term) of the encapsulated ChLcpVariables to zero.
	virtual void VariablesFbReset();

				/// Adds the current forces (applied to item) into the
				/// encapsulated ChLcpVariables, in the 'fb' part: qf+=forces*factor
	virtual void VariablesFbLoadForces(double factor=1.);

				/// Initialize the 'qb' part of the ChLcpVariables with the 
				/// current value of speeds. Note: since 'qb' is the unknown of the LCP, this
				/// function seems unuseful, unless used before VariablesFbIncrementMq()
	virtual void VariablesQbLoadSpeed();

				/// Adds M*q (masses multiplied current 'qb') to Fb, ex. if qb is initialized
				/// with v_old using VariablesQbLoadSpeed, this method can be used in 
				/// timestepping schemes that do: M*v_new = M*v_old + forces*dt
	virtual void VariablesFbIncrementMq();

				/// Fetches the item speed (ex. linear and angular vel.in rigid bodies) from the
				/// 'qb' part of the ChLcpVariables and sets it as the current item speed.
				/// If 'step' is not 0, also should compute the approximate acceleration of
				/// the item using backward differences, that is  accel=(new_speed-old_speed)/step.
				/// Mostly used after the LCP provided the solution in ChLcpVariables.
	virtual void VariablesQbSetSpeed(double step=0.);

				/// Increment item positions by the 'qb' part of the ChLcpVariables,
				/// multiplied by a 'step' factor.
				///     pos+=qb*step
				/// If qb is a speed, this behaves like a single step of 1-st order
				/// numerical integration (Eulero integration).
	virtual void VariablesQbIncrementPosition(double step);

				/// Tell to a system descriptor that there are variables of type
				/// ChLcpVariables in this object (for further passing it to a LCP solver)
				/// Basically does nothing, but maybe that inherited classes may specialize this.
	virtual void InjectVariables(ChLcpSystemDescriptor& mdescriptor);


};



} // END_OF_NAMESPACE____
} // END_OF_NAMESPACE____

#endif 
