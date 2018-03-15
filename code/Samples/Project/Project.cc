//------------------------------------------------------------------------------
//  Project.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/Main.h"
#include "Gfx/Gfx.h"
#include "Assets/Gfx/ShapeBuilder.h"
#include "Input/Input.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "shaders.h"
#include "deform.h"

using namespace Oryol;

class TriangleApp : public App {
public:
    AppState::Code OnRunning();
    AppState::Code OnInit();
    AppState::Code OnCleanup();

    DrawState drawState;
    DrawState drawState2;
    DrawState drawSpheres;
    DrawState drawLines;

    Shader::params params;
    glm::mat4 view;
    glm::mat4 proj;

    glm::vec3 c_position;
    glm::vec3 c_direction;
    glm::vec3 c_up;

    int forward = 0;
    int right = 0;

    bool cam_move = false;
    bool vert_move = false;

    float angleX = 0.0f;
    float angleY = 0.0f;
    float scaleFac = 1.f;
    Mesh testObject;

    bool mode = true;
    bool mode2 = true;

	bool ctrl = false;

	bool vert_mode = false;
	int sel_vert = 0;

    MeshSetup meshSetup;
    GfxSetup gfxSetup;
  
    int stab = 0;

    std::vector<int> selected;  
};
OryolMain(TriangleApp);

//------------------------------------------------------------------------------
AppState::Code
TriangleApp::OnInit() {

    this->c_position = glm::vec3(0, 0, 10);  
    this->c_direction = glm::vec3(0, 0, -1);  
    this->c_up = glm::vec3(0, 1, 0);  

    testObject.load_obj((char*)"../../../oryol/monkeh.obj");
    //testObject.load_obj((char*)"test.obj");
    testObject.generate_volume();
    
    weigh_nodes(&testObject);

    o_assert(testObject.init);

    gfxSetup = GfxSetup::Window(600, 400, "Deformate me!");

    gfxSetup.DefaultPassAction = PassAction::Clear(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
  
    // setup rendering system
    Gfx::Setup(gfxSetup);

//Draw ONE
    auto verts = testObject.get_vb();
    
    meshSetup = MeshSetup::Empty(verts.size(), Usage::Stream);

    meshSetup.Layout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Normal, VertexFormat::Float3 }
    };

    meshSetup.AddPrimitiveGroup(PrimitiveGroup(0, verts.size()*6));

    this->drawState.Mesh[0] = Gfx::CreateResource(meshSetup);
   
    //Gfx::UpdateVertices(this->drawState.Mesh[0], data, sizeof(float)*verts.size()*6);
    
    //Draw SPHERES
    
    ShapeBuilder sb;
    sb.RandomColors = true;
    sb.Layout = {
        { VertexAttr::Position, VertexFormat::Float3 },
        { VertexAttr::Normal, VertexFormat::Float3 }
    };
    
    sb.Sphere(0.02f, 9, 5);
    
    this->drawSpheres.Mesh[0] = Gfx::CreateResource(sb.Build());
    
    // create shader and pipeline-state-object
    Id shd = Gfx::CreateResource(Shader::Setup());
    auto ps = PipelineSetup::FromLayoutAndShader(meshSetup.Layout, shd);

    ps.DepthStencilState.DepthWriteEnabled = true;
    ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    ps.RasterizerState.SampleCount = gfxSetup.SampleCount;
    ps.PrimType = PrimitiveType::Triangles;
    this->drawState.Pipeline = Gfx::CreateResource(ps);

    auto ps2 = PipelineSetup::FromLayoutAndShader(meshSetup.Layout, shd);

    ps2.DepthStencilState.DepthWriteEnabled = true;
    ps2.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    ps2.RasterizerState.SampleCount = gfxSetup.SampleCount;
    this->drawSpheres.Pipeline = Gfx::CreateResource(ps2);

    const float fbWidth = (const float) Gfx::DisplayAttrs().FramebufferWidth;
    const float fbHeight = (const float) Gfx::DisplayAttrs().FramebufferHeight;
    this->proj = glm::perspectiveFov(glm::radians(45.0f), fbWidth, fbHeight, 0.01f, 100.0f);
    this->view = glm::mat4();

    Input::Setup();
    
    Input::SetPointerLockHandler([this](const InputEvent& e) -> PointerLockMode::Code {
            if (e.Button == MouseButton::Right) 
            {
                if (e.Type == InputEvent::MouseButtonDown) 
                {
                    // enable pointer-lock on LMB down
                    this->cam_move = true;
                    return PointerLockMode::Enable;
                }
                else if (e.Type == InputEvent::MouseButtonUp) 
                {
                    // ...and disable again when LMB released
                    this->cam_move = false;
                    return PointerLockMode::Disable;
                }
                else 
                {
                    // all other events: do nothing
                    return PointerLockMode::DontCare;
                }   
            }
	    else if(e.Button == MouseButton::Left)
	    { 
		if (e.Type == InputEvent::MouseButtonDown) 
                {
                    // enable pointer-lock on LMB down
                    this->vert_move = true;
                    return PointerLockMode::Enable;
                }
                else if (e.Type == InputEvent::MouseButtonUp) 
                {
                    // ...and disable again when LMB released
                    this->vert_move = false;
                    return PointerLockMode::Disable;
                }
                else 
                {
                    // all other events: do nothing
                    return PointerLockMode::DontCare;
                }
	    }
            else
                return PointerLockMode::DontCare;
        });

    
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
TriangleApp::OnRunning() {

    //this->angleY += 0.01f;
    //this->angleX += 0.02f;
  
    if(Input::MouseAttached())
    {
        const glm::vec2& mouseScroll = Input::MouseScroll();

        if(mouseScroll[1] > 0)
        {
            this->scaleFac += 0.1f;
            if(this->scaleFac > 10.f)
                this->scaleFac = 10.f;
        }
        else if(mouseScroll[1] < 0)
        {
            this->scaleFac -= 0.1f;
            if(this->scaleFac < 0.1f)
                this->scaleFac = 0.1f;
        }
	
        if(this->cam_move)
        {
            const glm::vec2& mouseMove = Input::MouseMovement();
	    
            this->angleY = mouseMove[0] / 200.f;
            this->angleX = mouseMove[1] / 200.f;
        }
        else
        {
            this->angleY = 0;
            this->angleX = 0;
        }

	//Selection of vertices.
	const glm::vec2& movePos = Input::MousePosition();
	//printf("MousePos: %f, %f\n", movePos[0], movePos[1]);
	
	if(Input::MouseButtonDown(MouseButton::Left))
	{
		clear_edges(selected);	
		if(selected.size() < 1)
		{
			glm::vec4 ray_dir = glm::vec4(c_direction, 0);
			ray_dir = ray_dir * glm::rotate(glm::mat4(), (movePos[0] - 300.f)/80.f, c_up);
			ray_dir = ray_dir * glm::rotate(glm::mat4(), (movePos[1] - 200.f)/60.f, glm::cross(c_direction, c_up));

			select_edge(&testObject, v3(c_position[0], c_position[1], c_position[2]), v3(ray_dir[0], ray_dir[1], ray_dir[2]), selected);
			
			if(selected.size() > 0)
				sel_vert = selected[0];
		}
	}

	if(vert_move)
	{	
		const glm::vec2& mouseMove = Input::MouseMovement();
		std::vector<v3> new_pos;

		glm::vec3 mv_t = glm::vec3(mouseMove, 0.f) * 0.0001f;
		
		v3 mv;

		if(!ctrl)
			mv = v3(mv_t[0], -mv_t[1], mv_t[2]);
		else
			mv = v3(mv_t[0], mv_t[2], mv_t[1]);

		new_pos.push_back(testObject.vert_buf[sel_vert].pos + mv);

		if(selected.size() == 0)
			selected.push_back(sel_vert);

		deform(&testObject, selected, new_pos);
	}
    }

		
    if(Input::KeyboardAttached()) 
    {
        if(Input::KeyDown(Key::W) || Input::KeyUp(Key::S)) 
        {
            this->forward++;
        }
        if(Input::KeyDown(Key::S) || Input::KeyUp(Key::W)) 
        {
            this->forward--;
        }

        if(Input::KeyDown(Key::D) || Input::KeyUp(Key::A)) 
        {
            this->right++;
        }
        if(Input::KeyDown(Key::A) || Input::KeyUp(Key::D)) 
        {
            this->right--;
        }
        if(Input::KeyDown(Key::X)) 
        {
            this->right = 0;
            this->forward = 0;
        }

	if(Input::KeyDown(Key::LeftControl))
	{
		ctrl = true;
	}
	
	if(Input::KeyUp(Key::LeftControl))
	{
		ctrl = false;
	}

        if(Input::KeyDown(Key::N1)) 
        {
            Id shd = Gfx::CreateResource(Shader::Setup());
            auto ps = PipelineSetup::FromLayoutAndShader(meshSetup.Layout, shd);
            
            ps.DepthStencilState.DepthWriteEnabled = true;
            ps.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
            ps.RasterizerState.SampleCount = gfxSetup.SampleCount;

            if(this->mode)
                ps.PrimType = PrimitiveType::Points;
            else
                ps.PrimType = PrimitiveType::Triangles;

            this->mode = !(this->mode);
            this->drawState.Pipeline = Gfx::CreateResource(ps);
        }
        
        if(Input::KeyDown(Key::M))
	{
		vert_mode = !vert_mode;
	}

        if(Input::KeyDown(Key::NumAdd) || Input::KeyPressed(Key::Num9))
	{
		if(sel_vert < testObject.vert_buf.size() - 1)
		{
			sel_vert++;
		}
	}
    
        if(Input::KeyDown(Key::NumSubtract) || Input::KeyPressed(Key::NumMultiply))
	{
		if(sel_vert > 0)
		{
			sel_vert--;
		}
	}

        if(Input::KeyDown(Key::Num0))
	       sel_vert = 0;	
        if(Input::KeyDown(Key::NumDecimal))
	       sel_vert = testObject.vert_buf.size() - 1;	

    }

    if(this->forward > 0)
    {
        this->c_position += this->c_direction * 0.05f;
    }
    if(this->forward < 0)
    {
        this->c_position -= this->c_direction * 0.05f;
    }

    if(this->right > 0)
    {
        this->c_position += glm::cross(this->c_direction, glm::vec3(0, 1, 0)) * 0.05f;
    }
    if(this->right < 0)
    {
        this->c_position -= glm::cross(this->c_direction, glm::vec3(0, 1, 0)) * 0.05f;
    }

    glm::vec4 camera = glm::vec4(this->c_direction, 1);

    camera = camera * glm::rotate(glm::mat4(), this->angleY, glm::vec3(0.0f, 1.0f, 0.0f));
    camera = camera * glm::rotate(glm::mat4(), -this->angleX, glm::vec3(1.0f, 0.0f, 0.0f));

    this->c_up = glm::vec3(glm::vec4(this->c_up, 1) * glm::rotate(glm::mat4(), this->angleY, glm::vec3(0.0f, 1.0f, 0.0f)));
    this->c_up = glm::vec3(glm::vec4(this->c_up, 1) * glm::rotate(glm::mat4(), this->angleX, glm::vec3(1.0f, 0.0f, 0.0f)));

    this->c_direction = glm::vec3(camera);
    
    this->view = glm::lookAt(
        this->c_position,
        this->c_position + this->c_direction,
        this->c_up
        );

    //Draw
    
    	auto verts = testObject.get_vb();
	   
	float * data = new float[verts.size()*6];
			
	for(int i = 0; i < verts.size(); i++)
	{
		data[i*6 + 0] = verts[i].pos[0];
		data[i*6 + 1] = verts[i].pos[1];
		data[i*6 + 2] = verts[i].pos[2];
		data[i*6 + 3] = verts[i].norm[0];
		data[i*6 + 4] = verts[i].norm[1];
		data[i*6 + 5] = verts[i].norm[2];
	}

	Gfx::UpdateVertices(this->drawState.Mesh[0], data, sizeof(float)*verts.size()*6);

    Gfx::BeginPass();
    Gfx::ApplyDrawState(this->drawSpheres);
    
    if(!vert_mode)
    {
	    for(uint i = 0; i < testObject.v_1.size(); i++)
	    {
		for(uint j = 0; j < testObject.v_1[0].size(); j++)
		{
		    for(uint k = 0; k < testObject.v_1[0][0].size(); k++)
		    {
			if(testObject.v_1[i][j][k] != nullptr)
			{
			    glm::vec3 pos(testObject.v_1[i][j][k]->pos[0] * (1.f / this->scaleFac),
					  testObject.v_1[i][j][k]->pos[1] * (1.f / this->scaleFac),
					  testObject.v_1[i][j][k]->pos[2] * (1.f / this->scaleFac)
				);

			    glm::mat4 model = glm::translate(glm::mat4(), pos);
		
			    this->params.mvp = this->proj * this->view * model;
			    Gfx::ApplyUniformBlock(this->params);
	    
			    Gfx::Draw();
			}
		    }
		}
	    }

	    for(uint i = 0; i < testObject.v_2.size(); i++)
	    {
		for(uint j = 0; j < testObject.v_2[0].size(); j++)
		{
		    for(uint k = 0; k < testObject.v_2[0][0].size(); k++)
		    {
			if(testObject.v_2[i][j][k] != nullptr)
			{
			    glm::vec3 pos(testObject.v_2[i][j][k]->pos[0] * (1.f / this->scaleFac),
					  testObject.v_2[i][j][k]->pos[1] * (1.f / this->scaleFac),
					  testObject.v_2[i][j][k]->pos[2] * (1.f / this->scaleFac)
				);

			    glm::mat4 model = glm::translate(glm::mat4(), pos);
		
			    this->params.mvp = this->proj * this->view * model;
			    Gfx::ApplyUniformBlock(this->params);
	    
			    Gfx::Draw();
			}
		    }
		}
	    }
    }
    else
    {
	    
	     	glm::vec3 pos(testObject.vert_buf[sel_vert].pos.x * (1.f / this->scaleFac),
			  testObject.vert_buf[sel_vert].pos.y * (1.f / this->scaleFac),
			  testObject.vert_buf[sel_vert].pos.z * (1.f / this->scaleFac)
	     	);

	    	glm::mat4 model = glm::translate(glm::mat4(), pos);

	    	this->params.mvp = this->proj * this->view * model *
			glm::scale(glm::mat4(), glm::vec3(2.f));
	    	Gfx::ApplyUniformBlock(this->params);

	    	Gfx::Draw();

	for(int i = 0; i < testObject.vert_buf[sel_vert].nbs.size(); i++)
   	{
		auto duvet = testObject.vert_buf[testObject.vert_buf[sel_vert].nbs[i]].pos;

	     	glm::vec3 pos(duvet.x * (1.f / this->scaleFac),
			  duvet.y * (1.f / this->scaleFac),
			  duvet.z * (1.f / this->scaleFac)
	     	);

	    	glm::mat4 model = glm::translate(glm::mat4(), pos);

	    	this->params.mvp = this->proj * this->view * model;
	    	Gfx::ApplyUniformBlock(this->params);

	    	Gfx::Draw();

	}
    }
    Gfx::ApplyDrawState(this->drawState);

    glm::mat4 modelTform = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f));

    this->params.mvp = this->proj * this->view * modelTform * glm::scale(glm::mat4(), glm::vec3(1.f / this->scaleFac));
    Gfx::ApplyUniformBlock(this->params);

    Gfx::Draw();

    /*this->params.mvp = this->proj * this->view * modelTform *
        glm::scale(glm::mat4(), glm::vec3(1.f / this->scaleFac));
    Gfx::ApplyUniformBlock(this->params);
    
    Gfx::Draw();*/
    Gfx::EndPass();
    Gfx::CommitFrame();
    
    delete[] data;

    // continue running or quit?
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
TriangleApp::OnCleanup() {
    Input::Discard();
    Gfx::Discard();
     
    return App::OnCleanup();
}
