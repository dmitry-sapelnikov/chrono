#pragma once

#pragma unmanaged
#include <irrlicht.h>
#include "physics/ChApidll.h" 
#include "physics/ChSystem.h"
#include "irrlicht_interface/ChBodySceneNode.h"
#include "irrlicht_interface/ChBodySceneNodeTools.h"
#include "irrlicht_interface/ChIrrWizard.h"
#include "irrlicht_interface/ChDisplayTools.h" 
#include "core/ChRealtimeStep.h"
#include "MyOscillatorSystem.h"
#pragma managed

// Use the namespace of Chrono

using namespace chrono;

// Use the main namespaces of Irrlicht

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
 
 




namespace oscillator {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			// Initialize all GUI items
			InitializeComponent();

			// Create the Irrlicht 3D device and bind it to a GUI widget

			int handlepointer = this->pictureBox1->Handle.ToInt32(); // will show into the pictureBox1
			irr::SIrrlichtCreationParameters param;
			param.WindowId = reinterpret_cast<void*>(handlepointer); 
			param.DriverType = video::EDT_OPENGL;
			//param.Stencilbuffer = true;
			this->aIrrDevice = irr::createDeviceEx(param);

			ChIrrWizard::add_typical_Logo(this->aIrrDevice);
			ChIrrWizard::add_typical_Sky(this->aIrrDevice);
			ChIrrWizard::add_typical_Lights(this->aIrrDevice);
			ChIrrWizard::add_typical_Camera(this->aIrrDevice, core::vector3df(2,1,-3));


			// Create Chrono::Engine items - adding them to the physical system

			chrono::DLL_CreateGlobals();  // important! before all uses of chrono engine items!

			this->oscillator = new MyOscillatorSystem(this->aIrrDevice);
	
			// Set starting values in GUI items, as the initialized oscillator
			this->numeric_mass->Value = Decimal(oscillator->my_body->GetBody()->GetMass());
			this->numeric_d0->Value = Decimal(oscillator->my_spring->Get_SpringRestLenght());
			this->numeric_K->Value = Decimal(oscillator->my_spring->Get_SpringK());
			this->numeric_R->Value = Decimal(oscillator->my_spring->Get_SpringR());
			this->numeric_X0->Value = Decimal(oscillator->X0);
			this->numeric_V0->Value = Decimal(oscillator->V0);
			this->numeric_dt->Value = Decimal(oscillator->physical_system.GetStep());
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{	
			// This safely delete every Irrlicht items, including the chrono bodies with 3d box shapes
			this->aIrrDevice->drop();

			// Delete all chrono global data - that's all with chrono. 
			chrono::DLL_DeleteGlobals();

			// this deletes the chrono physical system (also unreferences the added links)
			if (this->oscillator) delete this->oscillator;

			if (components)
			{
				delete components;
			}


		}

			//
			// User interface items (THIS SECTION IS AUTOMATICALLY GENERATED BY VISUAL STUDIO IDE)
			//
	private: System::Windows::Forms::NumericUpDown^  numeric_mass;
	protected: 
	private: System::Windows::Forms::Label^  label_mass;
	private: System::Windows::Forms::PictureBox^  pictureBox1;
	private: System::Windows::Forms::Label^  label_k;
	private: System::Windows::Forms::NumericUpDown^  numeric_K;
	private: System::Windows::Forms::Button^  button_start;

			//
			// Chrono::Engine data
			//
	MyOscillatorSystem* oscillator;

			//
			// Irrlicht 3D engine data
			//
	IrrlichtDevice* aIrrDevice;
	private: System::Windows::Forms::NumericUpDown^  numeric_R;
	private: System::Windows::Forms::Label^  label_r;
	private: System::Windows::Forms::NumericUpDown^  numeric_X0;
	private: System::Windows::Forms::Label^  label_x0;
	private: System::Windows::Forms::Label^  label_v0;
	private: System::Windows::Forms::NumericUpDown^  numeric_V0;
	private: System::Windows::Forms::NumericUpDown^  numeric_d0;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::ToolTip^  toolTip_mass;
	private: System::Windows::Forms::NumericUpDown^  numeric_realstep;
	private: System::Windows::Forms::Label^  label_realstep;
	private: System::Windows::Forms::Label^  label_dt;
	private: System::Windows::Forms::NumericUpDown^  numeric_dt;
	private: System::Windows::Forms::CheckBox^  checkBox_realtime;
	private: System::Windows::Forms::GroupBox^  group_step;
	private: System::Windows::Forms::GroupBox^  group_spring;

	private: System::Windows::Forms::GroupBox^  group_boundary;




	private: System::ComponentModel::IContainer^  components;





	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
			this->numeric_mass = (gcnew System::Windows::Forms::NumericUpDown());
			this->label_mass = (gcnew System::Windows::Forms::Label());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->label_k = (gcnew System::Windows::Forms::Label());
			this->numeric_K = (gcnew System::Windows::Forms::NumericUpDown());
			this->button_start = (gcnew System::Windows::Forms::Button());
			this->numeric_R = (gcnew System::Windows::Forms::NumericUpDown());
			this->label_r = (gcnew System::Windows::Forms::Label());
			this->numeric_X0 = (gcnew System::Windows::Forms::NumericUpDown());
			this->label_x0 = (gcnew System::Windows::Forms::Label());
			this->label_v0 = (gcnew System::Windows::Forms::Label());
			this->numeric_V0 = (gcnew System::Windows::Forms::NumericUpDown());
			this->numeric_d0 = (gcnew System::Windows::Forms::NumericUpDown());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->toolTip_mass = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->numeric_realstep = (gcnew System::Windows::Forms::NumericUpDown());
			this->numeric_dt = (gcnew System::Windows::Forms::NumericUpDown());
			this->checkBox_realtime = (gcnew System::Windows::Forms::CheckBox());
			this->label_realstep = (gcnew System::Windows::Forms::Label());
			this->label_dt = (gcnew System::Windows::Forms::Label());
			this->group_step = (gcnew System::Windows::Forms::GroupBox());
			this->group_spring = (gcnew System::Windows::Forms::GroupBox());
			this->group_boundary = (gcnew System::Windows::Forms::GroupBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_mass))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_K))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_R))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_X0))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_V0))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_d0))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_realstep))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_dt))->BeginInit();
			this->group_step->SuspendLayout();
			this->group_spring->SuspendLayout();
			this->group_boundary->SuspendLayout();
			this->SuspendLayout();
			// 
			// numeric_mass
			// 
			this->numeric_mass->DecimalPlaces = 4;
			this->numeric_mass->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 65536});
			this->numeric_mass->Location = System::Drawing::Point(79, 17);
			this->numeric_mass->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 196608});
			this->numeric_mass->Name = L"numeric_mass";
			this->numeric_mass->Size = System::Drawing::Size(84, 20);
			this->numeric_mass->TabIndex = 0;
			this->toolTip_mass->SetToolTip(this->numeric_mass, L"This is the mass of the oscillating particle, in Kg.\r\n");
			this->numeric_mass->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 196608});
			this->numeric_mass->ValueChanged += gcnew System::EventHandler(this, &Form1::numeric_mass_ValueChanged);
			// 
			// label_mass
			// 
			this->label_mass->AutoSize = true;
			this->label_mass->Location = System::Drawing::Point(13, 19);
			this->label_mass->Name = L"label_mass";
			this->label_mass->Size = System::Drawing::Size(54, 13);
			this->label_mass->TabIndex = 1;
			this->label_mass->Text = L"Mass [Kg]";
			// 
			// pictureBox1
			// 
			this->pictureBox1->Location = System::Drawing::Point(192, 17);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(449, 259);
			this->pictureBox1->TabIndex = 2;
			this->pictureBox1->TabStop = false;
			this->pictureBox1->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::pictureBox1_Paint);
			// 
			// label_k
			// 
			this->label_k->AutoSize = true;
			this->label_k->Location = System::Drawing::Point(17, 43);
			this->label_k->Name = L"label_k";
			this->label_k->Size = System::Drawing::Size(44, 13);
			this->label_k->TabIndex = 3;
			this->label_k->Text = L"K [N/m]";
			// 
			// numeric_K
			// 
			this->numeric_K->DecimalPlaces = 4;
			this->numeric_K->Location = System::Drawing::Point(67, 41);
			this->numeric_K->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1000, 0, 0, 0});
			this->numeric_K->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 196608});
			this->numeric_K->Name = L"numeric_K";
			this->numeric_K->Size = System::Drawing::Size(84, 20);
			this->numeric_K->TabIndex = 4;
			this->toolTip_mass->SetToolTip(this->numeric_K, L"This is the stiffness of the spring, expressed\r\nin Newtons per meter of elongatio" 
				L"n [N/m].\r\nNote: too high values might lead to numerical\r\ndivergence.\r\n\r\n");
			this->numeric_K->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 196608});
			this->numeric_K->ValueChanged += gcnew System::EventHandler(this, &Form1::numeric_K_ValueChanged);
			// 
			// button_start
			// 
			this->button_start->Location = System::Drawing::Point(15, 434);
			this->button_start->Name = L"button_start";
			this->button_start->Size = System::Drawing::Size(151, 23);
			this->button_start->TabIndex = 5;
			this->button_start->Text = L"START";
			this->toolTip_mass->SetToolTip(this->button_start, L"Start the simulation.");
			this->button_start->UseVisualStyleBackColor = true;
			this->button_start->Click += gcnew System::EventHandler(this, &Form1::button_start_Click);
			// 
			// numeric_R
			// 
			this->numeric_R->DecimalPlaces = 4;
			this->numeric_R->Location = System::Drawing::Point(67, 62);
			this->numeric_R->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1000, 0, 0, 0});
			this->numeric_R->Name = L"numeric_R";
			this->numeric_R->Size = System::Drawing::Size(84, 20);
			this->numeric_R->TabIndex = 6;
			this->toolTip_mass->SetToolTip(this->numeric_R, resources->GetString(L"numeric_R.ToolTip"));
			this->numeric_R->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {2, 0, 0, 196608});
			this->numeric_R->ValueChanged += gcnew System::EventHandler(this, &Form1::numeric_R_ValueChanged);
			// 
			// label_r
			// 
			this->label_r->AutoSize = true;
			this->label_r->Location = System::Drawing::Point(11, 64);
			this->label_r->Name = L"label_r";
			this->label_r->Size = System::Drawing::Size(50, 13);
			this->label_r->TabIndex = 7;
			this->label_r->Text = L"R [Ns/m]";
			// 
			// numeric_X0
			// 
			this->numeric_X0->DecimalPlaces = 4;
			this->numeric_X0->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 65536});
			this->numeric_X0->Location = System::Drawing::Point(67, 23);
			this->numeric_X0->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
			this->numeric_X0->Name = L"numeric_X0";
			this->numeric_X0->Size = System::Drawing::Size(84, 20);
			this->numeric_X0->TabIndex = 8;
			this->toolTip_mass->SetToolTip(this->numeric_X0, L"This is the initial horizontal position, expressed in meters [m],  \r\nof the movin" 
				L"g mass, at  the initial instant time=0.\r\nThis is a \'boundary condition\'.");
			this->numeric_X0->ValueChanged += gcnew System::EventHandler(this, &Form1::numeric_X0_ValueChanged);
			// 
			// label_x0
			// 
			this->label_x0->AutoSize = true;
			this->label_x0->Location = System::Drawing::Point(24, 25);
			this->label_x0->Name = L"label_x0";
			this->label_x0->Size = System::Drawing::Size(37, 13);
			this->label_x0->TabIndex = 9;
			this->label_x0->Text = L"X0 [m]";
			// 
			// label_v0
			// 
			this->label_v0->AutoSize = true;
			this->label_v0->Location = System::Drawing::Point(14, 46);
			this->label_v0->Name = L"label_v0";
			this->label_v0->Size = System::Drawing::Size(47, 13);
			this->label_v0->TabIndex = 11;
			this->label_v0->Text = L"V0 [m/s]";
			// 
			// numeric_V0
			// 
			this->numeric_V0->DecimalPlaces = 4;
			this->numeric_V0->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 65536});
			this->numeric_V0->Location = System::Drawing::Point(67, 44);
			this->numeric_V0->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
			this->numeric_V0->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, -2147483648});
			this->numeric_V0->Name = L"numeric_V0";
			this->numeric_V0->Size = System::Drawing::Size(84, 20);
			this->numeric_V0->TabIndex = 10;
			this->toolTip_mass->SetToolTip(this->numeric_V0, L"This is the initial horizontal speed, expressed in meters/second [m/s],  \r\nof the" 
				L" moving mass, at  the initial instant time=0.\r\nThis is a \'boundary condition\'.");
			this->numeric_V0->ValueChanged += gcnew System::EventHandler(this, &Form1::numeric_v0_ValueChanged);
			// 
			// numeric_d0
			// 
			this->numeric_d0->DecimalPlaces = 4;
			this->numeric_d0->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 65536});
			this->numeric_d0->Location = System::Drawing::Point(67, 20);
			this->numeric_d0->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
			this->numeric_d0->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 131072});
			this->numeric_d0->Name = L"numeric_d0";
			this->numeric_d0->Size = System::Drawing::Size(84, 20);
			this->numeric_d0->TabIndex = 12;
			this->toolTip_mass->SetToolTip(this->numeric_d0, L"This is the length of the undeformed spring\r\n(i.e. the length of the spring witho" 
				L"ut any applied\r\nforces, at rest). It is expressed in meters [m]\r\n  ");
			this->numeric_d0->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 131072});
			this->numeric_d0->ValueChanged += gcnew System::EventHandler(this, &Form1::numeric_d0_ValueChanged);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(25, 22);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(36, 13);
			this->label1->TabIndex = 13;
			this->label1->Text = L"d0 [m]";
			// 
			// numeric_realstep
			// 
			this->numeric_realstep->BackColor = System::Drawing::SystemColors::ActiveBorder;
			this->numeric_realstep->DecimalPlaces = 6;
			this->numeric_realstep->ForeColor = System::Drawing::SystemColors::AppWorkspace;
			this->numeric_realstep->Location = System::Drawing::Point(67, 68);
			this->numeric_realstep->Name = L"numeric_realstep";
			this->numeric_realstep->ReadOnly = true;
			this->numeric_realstep->Size = System::Drawing::Size(84, 20);
			this->numeric_realstep->TabIndex = 14;
			this->toolTip_mass->SetToolTip(this->numeric_realstep, resources->GetString(L"numeric_realstep.ToolTip"));
			// 
			// numeric_dt
			// 
			this->numeric_dt->DecimalPlaces = 6;
			this->numeric_dt->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 196608});
			this->numeric_dt->Location = System::Drawing::Point(67, 47);
			this->numeric_dt->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1, 0, 0, 65536});
			this->numeric_dt->Name = L"numeric_dt";
			this->numeric_dt->Size = System::Drawing::Size(84, 20);
			this->numeric_dt->TabIndex = 16;
			this->toolTip_mass->SetToolTip(this->numeric_dt, resources->GetString(L"numeric_dt.ToolTip"));
			// 
			// checkBox_realtime
			// 
			this->checkBox_realtime->AutoSize = true;
			this->checkBox_realtime->CheckAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->checkBox_realtime->Checked = true;
			this->checkBox_realtime->CheckState = System::Windows::Forms::CheckState::Checked;
			this->checkBox_realtime->Location = System::Drawing::Point(11, 24);
			this->checkBox_realtime->Name = L"checkBox_realtime";
			this->checkBox_realtime->Size = System::Drawing::Size(70, 17);
			this->checkBox_realtime->TabIndex = 18;
			this->checkBox_realtime->Text = L"Real time";
			this->toolTip_mass->SetToolTip(this->checkBox_realtime, resources->GetString(L"checkBox_realtime.ToolTip"));
			this->checkBox_realtime->UseVisualStyleBackColor = true;
			this->checkBox_realtime->CheckedChanged += gcnew System::EventHandler(this, &Form1::checkBox_realtime_CheckedChanged);
			// 
			// label_realstep
			// 
			this->label_realstep->AutoSize = true;
			this->label_realstep->Location = System::Drawing::Point(0, 70);
			this->label_realstep->Name = L"label_realstep";
			this->label_realstep->Size = System::Drawing::Size(61, 13);
			this->label_realstep->TabIndex = 15;
			this->label_realstep->Text = L"Effective dt";
			// 
			// label_dt
			// 
			this->label_dt->AutoSize = true;
			this->label_dt->Location = System::Drawing::Point(8, 49);
			this->label_dt->Name = L"label_dt";
			this->label_dt->Size = System::Drawing::Size(53, 13);
			this->label_dt->TabIndex = 17;
			this->label_dt->Text = L"Max dt [s]";
			// 
			// group_step
			// 
			this->group_step->Controls->Add(this->checkBox_realtime);
			this->group_step->Controls->Add(this->label_realstep);
			this->group_step->Controls->Add(this->label_dt);
			this->group_step->Controls->Add(this->numeric_realstep);
			this->group_step->Controls->Add(this->numeric_dt);
			this->group_step->Location = System::Drawing::Point(12, 224);
			this->group_step->Name = L"group_step";
			this->group_step->Size = System::Drawing::Size(162, 96);
			this->group_step->TabIndex = 19;
			this->group_step->TabStop = false;
			this->group_step->Text = L"Step control";
			// 
			// group_spring
			// 
			this->group_spring->Controls->Add(this->numeric_d0);
			this->group_spring->Controls->Add(this->label1);
			this->group_spring->Controls->Add(this->label_k);
			this->group_spring->Controls->Add(this->numeric_K);
			this->group_spring->Controls->Add(this->numeric_R);
			this->group_spring->Controls->Add(this->label_r);
			this->group_spring->Location = System::Drawing::Point(12, 123);
			this->group_spring->Name = L"group_spring";
			this->group_spring->Size = System::Drawing::Size(162, 95);
			this->group_spring->TabIndex = 20;
			this->group_spring->TabStop = false;
			this->group_spring->Text = L"Spring-dashpot";
			// 
			// group_boundary
			// 
			this->group_boundary->Controls->Add(this->numeric_X0);
			this->group_boundary->Controls->Add(this->label_x0);
			this->group_boundary->Controls->Add(this->numeric_V0);
			this->group_boundary->Controls->Add(this->label_v0);
			this->group_boundary->Location = System::Drawing::Point(12, 43);
			this->group_boundary->Name = L"group_boundary";
			this->group_boundary->Size = System::Drawing::Size(162, 74);
			this->group_boundary->TabIndex = 21;
			this->group_boundary->TabStop = false;
			this->group_boundary->Text = L"Boundary conditions";
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(653, 516);
			this->Controls->Add(this->group_boundary);
			this->Controls->Add(this->group_spring);
			this->Controls->Add(this->button_start);
			this->Controls->Add(this->pictureBox1);
			this->Controls->Add(this->label_mass);
			this->Controls->Add(this->numeric_mass);
			this->Controls->Add(this->group_step);
			this->Name = L"Form1";
			this->Text = L"Chrono::Engine within a  Windows Forms interface";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_mass))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_K))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_R))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_X0))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_V0))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_d0))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_realstep))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numeric_dt))->EndInit();
			this->group_step->ResumeLayout(false);
			this->group_step->PerformLayout();
			this->group_spring->ResumeLayout(false);
			this->group_spring->PerformLayout();
			this->group_boundary->ResumeLayout(false);
			this->group_boundary->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
		
	void redraw_3d_window() 
			 {
				this->aIrrDevice->getVideoDriver()->beginScene(true, true, SColor(255,150,153,158));
				this->aIrrDevice->getSceneManager()->drawAll();
				this->aIrrDevice->getGUIEnvironment()->drawAll();
				ChIrrTools::drawGrid(this->aIrrDevice->getVideoDriver(), 1, 1);
				ChIrrTools::drawSpring(this->aIrrDevice->getVideoDriver(), 0.1, this->oscillator->my_spring->GetEndPoint1Abs(), this->oscillator->my_spring->GetEndPoint2Abs() );
				this->aIrrDevice->getVideoDriver()->endScene();
			 }

	private: System::Void button_start_Click(System::Object^  sender, System::EventArgs^  e) 
			 {
				 ChRealtimeStepTimer m_realtime_timer;
				 bool mrealtime = this->checkBox_realtime->Checked;
				 double maxstep = Decimal::ToDouble(this->numeric_dt->Value);

				 this->oscillator->ResetToInitialState(); // restores X0, V0 initial positions
				 
				 // COMPUTE SIMULATION
				 for(int i=0; i< 2000; i++)
				 {
					double mstep;
					if (mrealtime)
						mstep = m_realtime_timer.SuggestSimulationStep(maxstep);
					else 
						mstep = maxstep;
					this->oscillator->physical_system.DoStepDynamics(mstep);
					this->redraw_3d_window();
				 }
			 }

	private: System::Void pictureBox1_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) 
			 {
				this->redraw_3d_window();
			 }

	private: System::Void numeric_mass_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
			 {
				 this->oscillator->my_body->GetBody()->SetMass( Decimal::ToDouble(this->numeric_mass->Value) );
			 }
private: System::Void numeric_K_ValueChanged(System::Object^  sender, System::EventArgs^  e) 			 
			 {
				 this->oscillator->my_spring->Set_SpringK( Decimal::ToDouble(this->numeric_K->Value) );
			 }
private: System::Void numeric_R_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
			 {
				 this->oscillator->my_spring->Set_SpringR( Decimal::ToDouble(this->numeric_R->Value) );
			 }
private: System::Void numeric_X0_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
			 {
				 this->oscillator->X0 = Decimal::ToDouble(this->numeric_X0->Value);
				 this->oscillator->ResetToInitialState();
				 this->redraw_3d_window();
			 }
private: System::Void numeric_v0_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
			 {
				 this->oscillator->V0 = Decimal::ToDouble(this->numeric_V0->Value);
		     }
private: System::Void numeric_d0_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
			 {
				 this->oscillator->my_spring->Set_SpringRestLenght ( Decimal::ToDouble(this->numeric_d0->Value) );
			 }

private: System::Void checkBox_realtime_CheckedChanged(System::Object^  sender, System::EventArgs^  e) 
			 {
				  if (this->checkBox_realtime->Checked)
				  {
					  this->label_dt->Text = "Max dt [s]";
					  this->numeric_realstep->Visible = true;
					  this->label_realstep->Visible = true;
				  }
				  else 
				  {
					  this->label_dt->Text = "dt [s]";
					  this->numeric_realstep->Visible = false;
					  this->label_realstep->Visible = false;
				  }
			 }
};
}

