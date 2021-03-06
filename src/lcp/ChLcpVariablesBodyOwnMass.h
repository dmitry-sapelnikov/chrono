//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010 Alessandro Tasora
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be 
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

#ifndef CHLCPVARIABLESBODYOWNMASS_H
#define CHLCPVARIABLESBODYOWNMASS_H

//////////////////////////////////////////////////
//
//   ChLcpVariablesBodyOwnMass.h
//
//    Specialized class for representing a mass matrix
//   and associate variables (6 element vector, ex.speed)
//   for a 3D rigid body.
//
//   HEADER file for CHRONO HYPEROCTANT LCP solver
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////


#include "ChLcpVariablesBody.h"


namespace chrono
{

///    Specialized class for representing a 6-DOF item for a
///   LCP system, that is a 3D rigid body, with mass matrix and
///   associate variables (a 6 element vector, ex.speed)
///    Differently from the 'naive' implementation ChLcpVariablesGeneric,
///   here a full 6x6 mass matrix is not built, since only the 3x3
///   inertia matrix and the mass value are enough.

class ChApi ChLcpVariablesBodyOwnMass :  public ChLcpVariablesBody
{
	CH_RTTI(ChLcpVariablesBodyOwnMass, ChLcpVariablesBody)

private:
			//
			// DATA
			//
				/// the data (qb, variables and fb, forces, already defined in base class)

			ChMatrix33<double> inertia;	// 3x3 inertia matrix
			double mass;		// mass value

			ChMatrix33<double> inv_inertia;
			double inv_mass;

public:

			//
			// CONSTRUCTORS
			//

	ChLcpVariablesBodyOwnMass() 
				{
					inertia.Set33Identity();
					inv_inertia.Set33Identity();
					mass = 1.;
					inv_mass = 1.;
				};

	virtual ~ChLcpVariablesBodyOwnMass()
				{
				};


				/// Assignment operator: copy from other object
	ChLcpVariablesBodyOwnMass& operator=(const ChLcpVariablesBodyOwnMass& other);


			//
			// FUNCTIONS
			//

				/// Get the mass associated with translation of body
	virtual double	GetBodyMass()	 {return mass;}

				/// Access the 3x3 inertia matrix
	virtual ChMatrix33<>& GetBodyInertia() {return inertia;}

				/// Access the 3x3 inertia matrix inverted
	ChMatrix33<>& GetBodyInvInertia() {return inv_inertia;}



				/// Set the inertia matrix
	void    SetBodyInertia(const ChMatrix33<>* minertia)
						{
							inertia.CopyFromMatrix(*minertia);
							inertia.FastInvert(&inv_inertia);
						}

				/// Set the mass associated with translation of body
	void    SetBodyMass(const double mmass)
						{
							mass = mmass;
							inv_mass = 1.0 / mass;
						}


				// IMPLEMENT PARENT CLASS METHODS



				/// Computes the product of the inverse mass matrix by a
				/// vector, and set in result: result = [invMb]*vect
	virtual void Compute_invMb_v(ChMatrix<float>& result, const ChMatrix<float>& vect)
					{
						assert (vect.GetRows()   == Get_ndof());
						assert (result.GetRows() == Get_ndof());
						// optimized unrolled operations
						result(0)= (float)inv_mass * vect(0);
						result(1)= (float)inv_mass * vect(1);
						result(2)= (float)inv_mass * vect(2);
						result(3)= (float)(inv_inertia(0,0)*vect(3) + inv_inertia(0,1)*vect(4) + inv_inertia(0,2)*vect(5));
						result(4)= (float)(inv_inertia(1,0)*vect(3) + inv_inertia(1,1)*vect(4) + inv_inertia(1,2)*vect(5));
						result(5)= (float)(inv_inertia(2,0)*vect(3) + inv_inertia(2,1)*vect(4) + inv_inertia(2,2)*vect(5));
					};
	virtual void Compute_invMb_v(ChMatrix<double>& result, const ChMatrix<double>& vect)
					{
						assert (vect.GetRows()   == Get_ndof());
						assert (result.GetRows() == Get_ndof());
						// optimized unrolled operations
						result(0)= inv_mass * vect(0);
						result(1)= inv_mass * vect(1);
						result(2)= inv_mass * vect(2);
						result(3)= inv_inertia(0,0)*vect(3) + inv_inertia(0,1)*vect(4) + inv_inertia(0,2)*vect(5);
						result(4)= inv_inertia(1,0)*vect(3) + inv_inertia(1,1)*vect(4) + inv_inertia(1,2)*vect(5);
						result(5)= inv_inertia(2,0)*vect(3) + inv_inertia(2,1)*vect(4) + inv_inertia(2,2)*vect(5);
					};

				/// Computes the product of the inverse mass matrix by a
				/// vector, and increment result: result += [invMb]*vect
	virtual void Compute_inc_invMb_v(ChMatrix<float>& result, const ChMatrix<float>& vect)
					{
						assert (vect.GetRows()   == Get_ndof());
						assert (result.GetRows() == Get_ndof());
						// optimized unrolled operations
						result(0)+= (float)inv_mass * vect(0);
						result(1)+= (float)inv_mass * vect(1);
						result(2)+= (float)inv_mass * vect(2);
						result(3)+= (float)(inv_inertia(0,0)*vect(3) + inv_inertia(0,1)*vect(4) + inv_inertia(0,2)*vect(5));
						result(4)+= (float)(inv_inertia(1,0)*vect(3) + inv_inertia(1,1)*vect(4) + inv_inertia(1,2)*vect(5));
						result(5)+= (float)(inv_inertia(2,0)*vect(3) + inv_inertia(2,1)*vect(4) + inv_inertia(2,2)*vect(5));
					};
	virtual void Compute_inc_invMb_v(ChMatrix<double>& result, const ChMatrix<double>& vect)
					{
						assert (vect.GetRows()   == Get_ndof());
						assert (result.GetRows() == Get_ndof());
						// optimized unrolled operations
						result(0)+= inv_mass * vect(0);
						result(1)+= inv_mass * vect(1);
						result(2)+= inv_mass * vect(2);
						result(3)+= inv_inertia(0,0)*vect(3) + inv_inertia(0,1)*vect(4) + inv_inertia(0,2)*vect(5);
						result(4)+= inv_inertia(1,0)*vect(3) + inv_inertia(1,1)*vect(4) + inv_inertia(1,2)*vect(5);
						result(5)+= inv_inertia(2,0)*vect(3) + inv_inertia(2,1)*vect(4) + inv_inertia(2,2)*vect(5);
					};


				/// Computes the product of the mass matrix by a
				/// vector, and set in result: result = [Mb]*vect
	virtual void Compute_inc_Mb_v(ChMatrix<float>& result, const ChMatrix<float>& vect)
					{
						assert (result.GetRows() == Get_ndof());
						assert (vect.GetRows()   == Get_ndof());
						// optimized unrolled operations
						result(0)+= (float)mass * vect(0);
						result(1)+= (float)mass * vect(1);
						result(2)+= (float)mass * vect(2);
						result(3)+= (float)(inertia(0,0)*vect(3) + inertia(0,1)*vect(4) + inertia(0,2)*vect(5));
						result(4)+= (float)(inertia(1,0)*vect(3) + inertia(1,1)*vect(4) + inertia(1,2)*vect(5));
						result(5)+= (float)(inertia(2,0)*vect(3) + inertia(2,1)*vect(4) + inertia(2,2)*vect(5));
					};
	virtual void Compute_inc_Mb_v(ChMatrix<double>& result, const ChMatrix<double>& vect)
					{
						assert (result.GetRows() == vect.GetRows());
						assert (vect.GetRows()   == Get_ndof());
						// optimized unrolled operations
						result(0)+= mass * vect(0);
						result(1)+= mass * vect(1);
						result(2)+= mass * vect(2);
						result(3)+= (inertia(0,0)*vect(3) + inertia(0,1)*vect(4) + inertia(0,2)*vect(5));
						result(4)+= (inertia(1,0)*vect(3) + inertia(1,1)*vect(4) + inertia(1,2)*vect(5));
						result(5)+= (inertia(2,0)*vect(3) + inertia(2,1)*vect(4) + inertia(2,2)*vect(5));
					};

				/// Computes the product of the corresponding block in the 
				/// system matrix (ie. the mass matrix) by 'vect', and add to 'result'. 
				/// NOTE: the 'vect' and 'result' vectors must already have
				/// the size of the total variables&constraints in the system; the procedure
				/// will use the ChVariable offsets (that must be already updated) to know the 
				/// indexes in result and vect.
	virtual void MultiplyAndAdd(ChMatrix<double>& result, const ChMatrix<double>& vect) const
					{
						assert(result.GetColumns()==1 && vect.GetColumns()==1);
						// optimized unrolled operations
						double q0 = vect(this->offset+0);
						double q1 = vect(this->offset+1);
						double q2 = vect(this->offset+2);
						double q3 = vect(this->offset+3);
						double q4 = vect(this->offset+4);
						double q5 = vect(this->offset+5);
						result(this->offset+0)+= mass * q0;
						result(this->offset+1)+= mass * q1;
						result(this->offset+2)+= mass * q2;
						result(this->offset+3)+= (inertia(0,0)*q3 + inertia(0,1)*q4 + inertia(0,2)*q5);
						result(this->offset+4)+= (inertia(1,0)*q3 + inertia(1,1)*q4 + inertia(1,2)*q5);
						result(this->offset+5)+= (inertia(2,0)*q3 + inertia(2,1)*q4 + inertia(2,2)*q5);
					}
	
				/// Add the diagonal of the mass matrix (as a column vector) to 'result'.
				/// NOTE: the 'result' vector must already have the size of system unknowns, ie
				/// the size of the total variables&constraints in the system; the procedure
				/// will use the ChVariable offset (that must be already updated) as index.
	virtual void DiagonalAdd(ChMatrix<double>& result) const 
					{
						assert(result.GetColumns()==1);
						result(this->offset+0)+= mass;
						result(this->offset+1)+= mass;
						result(this->offset+2)+= mass;
						result(this->offset+3)+= inertia(0,0);
						result(this->offset+4)+= inertia(1,1);
						result(this->offset+5)+= inertia(2,2);
					}

				/// Build the mass matrix (for these variables) storing
				/// it in 'storage' sparse matrix, at given column/row offset.
				/// Note, most iterative solvers don't need to know mass matrix explicitly.
				/// Optimised: doesn't fill unneeded elements except mass and 3x3 inertia.
	virtual void Build_M(ChSparseMatrix& storage, int insrow, int inscol)
					{
						storage.SetElement(insrow+0, inscol+0, mass);
						storage.SetElement(insrow+1, inscol+1, mass);
						storage.SetElement(insrow+2, inscol+2, mass);
						storage.PasteMatrix(&inertia, insrow+3, inscol+3);
					};


};




} // END_OF_NAMESPACE____




#endif  
