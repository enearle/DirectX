//***************************************************************************************
// TexColumnsApp.cpp by Frank Luna (C) 2015 All Rights Reserved. Moved to this from shapes app, then added every thing else. :D
//***************************************************************************************

#include "../../Common/d3dApp.h"
#include "../../Common/MathHelper.h"
#include "../../Common/UploadBuffer.h"
#include "../../Common/GeometryGenerator.h"
#include "FrameResource.h"
//Camera: Step 0
#include "../../Common/Camera.h"
#include "Labyrinth/LabyrinthGen.h"
#include "Physics/PhysicsEngine.h"
#include "GpuWaves.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = 3;

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
	RenderItem() = default;
    RenderItem(const RenderItem& rhs) = delete;

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Used for GPU waves render items.
	DirectX::XMFLOAT2 DisplacementMapTexelSize = { 1.0f, 1.0f };
	float GridSpatialStep = 1.0f;

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
};

// Alpha: step 1
// Trees: step 1
enum class RenderLayer : int
{
	Opaque = 0,
	Transparent,
	AlphaTested,
	AlphaTestedTreeSprites,
	GpuWaves,
	Count
};

class RigidGameObject
{
	
public:
	RigidGameObject(int bodyIndex, XMFLOAT3 framePosition) : bodyIndex(bodyIndex), framePosition(framePosition) {}
	int bodyIndex = 0;
	DirectX::XMFLOAT3 framePosition = {};

	void Update(float physicsTimer, float physicsDuration)
	{
		framePosition = PhysicsEngine::Instance->bodiesA[bodyIndex].position;
		//XMVectorLerp(XMLoadFloat3(&PhysicsEngine::Instance->bodiesA[bodyIndex].position), XMVectorAdd(XMLoadFloat3(&PhysicsEngine::Instance->bodiesA[bodyIndex].position), XMLoadFloat3(&PhysicsEngine::Instance->bodiesA[bodyIndex].velocity)), 0);
	};
	
};

class PlayerGameObject : public RigidGameObject
{
public:
	PlayerGameObject(int bodyIndex, XMFLOAT3 framePosition, float moveSpeed = 10) : RigidGameObject(bodyIndex, framePosition), moveSpeed(moveSpeed) {}

	bool w = false;
	bool s = false;
	bool a = false;
	bool d = false;
	float moveSpeed = 10;
};

class ShapesApp : public D3DApp
{
public:
	ShapesApp(HINSTANCE hInstance);
	ShapesApp(const ShapesApp& rhs) = delete;
	ShapesApp& operator=(const ShapesApp& rhs) = delete;
	~ShapesApp();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void AnimateMaterials(const GameTimer& gt);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdatePhysicsTimer(const GameTimer& gt);
	void UpdateGameObjects(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateWavesGPU(const GameTimer& gt);

	void BuildLabyrinth();
	void InitializePhysicsEngineAndGameObjects();
	void BuildWavesRootSignature();
	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildShapeGeometry();
	void BuildWavesGeometry();
	void BuildTreeSpritesGeometry(); //Trees: Step 1.2
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	SubmeshGeometry CreateSubmesh(GeometryGenerator::MeshData mesh, UINT& curVertCount, UINT& curIndCount);
	void AppendVerticesAndIndices(std::vector<Vertex>& vertices, std::vector<std::uint16_t>& indices, GeometryGenerator::MeshData mesh, UINT& curOffset, XMFLOAT4 color);

	//Trees: Step 2.2 (already implemented)
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:

	XMFLOAT3 StartLocation = {-50,100,-260};
	float moveSpeed = 10;
	int playerObjectIndex = 0;
	float physicsTimer = 0;
	float physicsFrameDuration = 0.01667;
	std::vector<RigidGameObject> gameObjects;
	std::vector<XMFLOAT3> labyrinth;

	// Camera before physics
	DirectX::XMFLOAT3 mRight = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 mUp = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 mLook = { 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 mLastRight = { 1.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 mLastUp = { 0.0f, 1.0f, 0.0f };
	DirectX::XMFLOAT3 mLastLook = { 0.0f, 0.0f, 1.0f };

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    UINT mCbvSrvDescriptorSize = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12RootSignature> mWavesRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;

	//Alpha: step 2 - create array of multiple pipeline state objects
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	
	//Trees: step 7
	std::vector<D3D12_INPUT_ELEMENT_DESC> mTreeSpriteInputLayout;
 
	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	std::unique_ptr<GpuWaves> mWaves;

	//Alpha: step 3
	//Render items divided by PSO.
	//std::vector<RenderItem*> mOpaqueRitems;
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

    PassConstants mMainPassCB;

	bool mIsWireframe = false;

	//Camera: step 1
	//XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	//XMFLOAT4X4 mView = MathHelper::Identity4x4();
	//XMFLOAT4X4 mProj = MathHelper::Identity4x4();
	//float mTheta = 1.5f * XM_PI;
	//float mPhi = 0.2f * XM_PI;
	//float mRadius = 15.0f;
	
	Camera mCamera;

    POINT mLastMousePos;
};



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
                   PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        ShapesApp theApp(hInstance);
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

ShapesApp::ShapesApp(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

ShapesApp::~ShapesApp()
{
    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool ShapesApp::Initialize()
{
    if(!D3DApp::Initialize())
        return false;

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    // Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
    mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	mWaves = std::make_unique<GpuWaves>(
	md3dDevice.Get(), 
	mCommandList.Get(),
	256, 256, 0.25f, 0.03f, 2.0f, 0.2f);
	
	//Camera: step 8 Initialize camera
	mCamera.SetPosition(StartLocation);
	
	LoadTextures();
    BuildRootSignature();
	BuildWavesRootSignature();
	BuildDescriptorHeaps();
    BuildShadersAndInputLayout();
    BuildShapeGeometry();
	BuildWavesGeometry();
	BuildTreeSpritesGeometry(); //Trees: Step 2
	BuildMaterials();
	BuildLabyrinth();
	InitializePhysicsEngineAndGameObjects();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}
 
void ShapesApp::OnResize()
{
	D3DApp::OnResize();

	//Camera: step 2
	
	//XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	//XMStoreFloat4x4(&mProj, P);

	mCamera.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void ShapesApp::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);
	UpdatePhysicsTimer(gt);
	//UpdateGameObjects(gt);
	UpdateCamera(gt);

    // Cycle through the circular frame resource array.
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }
	
	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
}

void ShapesApp::Draw(const GameTimer& gt)
{
    auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	UpdateWavesGPU(gt);
	mCommandList->SetPipelineState(mPSOs["opaque"].Get());
	
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	//ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	//mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	mCommandList->SetGraphicsRootDescriptorTable(4, mWaves->DisplacementMap());

	//Alpha: step 5
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["alphaTested"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTested]);

	//Trees: step 14? IDK anymore
	mCommandList->SetPipelineState(mPSOs["treeSprites"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites]);

	mCommandList->SetPipelineState(mPSOs["transparent"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Transparent]);

	mCommandList->SetPipelineState(mPSOs["wavesRender"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::GpuWaves]);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    mCurrFrameResource->Fence = ++mCurrentFence;

    // Add an instruction to the command queue to set a new fence point. 
    // Because we are on the GPU timeline, the new fence point won't be 
    // set until the GPU finishes processing all the commands prior to this Signal().
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void ShapesApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void ShapesApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void ShapesApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		//Camera: Step 4
		//mCamera.Pitch(dy);
		//mCamera.RotateY(dx);


		XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), dy);
		XMStoreFloat3(&mUp,   XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
		XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
		R = XMMatrixRotationY(dx);
		XMStoreFloat3(&mRight,   XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
		XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
		XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
		
		//PlayerGameObject* player = nullptr;
		//if(gameObjects.size() > playerObjectIndex)
		//	player = static_cast<PlayerGameObject*>(&gameObjects[playerObjectIndex]);
		//if(player)
		//{
		//	XMStoreFloat3(&PhysicsEngine::Instance->bodiesA[playerObjectIndex].forwardDirection, XMVector3Cross(XMVectorSet(0,1,0,0), mCamera.GetRight()));
		//}
		
		// Update angles based on input to orbit camera around box.
		//mTheta += dx;
		//mPhi += dy;

		// Restrict the angle mPhi.
		//mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	//else if ((btnState & MK_RBUTTON) != 0)
	//{
	//	// Make each pixel correspond to 0.2 unit in the scene.
	//	float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
	//	float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);
	//	// Update the camera radius based on input.
	//	mRadius += dx - dy;
	//	// Restrict the radius.
	//	mRadius = MathHelper::Clamp(mRadius, 5.0f, 150.0f);
	//}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
 
void ShapesApp::OnKeyboardInput(const GameTimer& gt)
{
	if (GetAsyncKeyState('1') & 0x8000)
		mIsWireframe = true;
	else
		mIsWireframe = false;

	const float dt = gt.DeltaTime();

	//Camera: step 3 (input polling)
	// GetAsyncKeyState returns a short (2 bytes)
	// using bitwise AND on the most significant bit, 1 is returned when key is pressed
	// 1000 0000 0000 0000 0000 0000 0000 0000		==		(16 ^ 3) * 8		==		any negative (signed) short, including -0
	// likely returns -0 everytime for byte alignment and fast access
	PlayerGameObject* player = nullptr;
	if(gameObjects.size() > playerObjectIndex)
		player = static_cast<PlayerGameObject*>(&gameObjects[playerObjectIndex]);
	if(player)
	{
		if(GetAsyncKeyState('W') & 0x8000) 
			player->w = true;
		if(GetAsyncKeyState('S') & 0x8000)
			player->s = true;
		if(GetAsyncKeyState('A') & 0x8000)
			player->a = true;
		if(GetAsyncKeyState('D') & 0x8000)
			player->d = true;
	}
	
	//if(GetAsyncKeyState('W') & 0x8000) 
	//	mCamera.Walk(10.0f*dt);
	//
	//if(GetAsyncKeyState('S') & 0x8000)
	//	mCamera.Walk(-10.0f*dt);
	//
	//if(GetAsyncKeyState('A') & 0x8000)
	//	mCamera.Strafe(-10.0f*dt);
	//
	//if(GetAsyncKeyState('D') & 0x8000)
	//	mCamera.Strafe(10.0f*dt);
	//
	//mCamera.UpdateViewMatrix();
}
 
void ShapesApp::UpdateCamera(const GameTimer& gt)
{
	XMFLOAT3 pos = {};
	if(gameObjects.size() > playerObjectIndex)
		pos = PhysicsEngine::Instance->bodiesA[playerObjectIndex].position;
	pos.y = pos.y + 1;
	mCamera.SetPosition(pos);
	mCamera.UpdateViewMatrix();
	// Camera: step 5 (remove UpdateCamera())
	// Convert Spherical to Cartesian coordinates.
	//mEyePos.x = mRadius*sinf(mPhi)*cosf(mTheta);
	//mEyePos.z = mRadius*sinf(mPhi)*sinf(mTheta);
	//mEyePos.y = mRadius*cosf(mPhi);
	//// Build the view matrix.
	//XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	//XMVECTOR target = XMVectorZero();
	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	//XMStoreFloat4x4(&mView, view);
}

void ShapesApp::AnimateMaterials(const GameTimer& gt)
{
	// Scroll the water material texture coordinates.
	auto waterMat = mMaterials["water0"].get();

	float& tu = waterMat->MatTransform(3, 0);
	float& tv = waterMat->MatTransform(3, 1);

	tu += 0.1f * gt.DeltaTime();
	tv += 0.02f * gt.DeltaTime();

	if(tu >= 1.0f)
		tu -= 1.0f;

	if(tv >= 1.0f)
		tv -= 1.0f;

	waterMat->MatTransform(3, 0) = tu;
	waterMat->MatTransform(3, 1) = tv;

	// Material has changed, so need to update cbuffer.
	waterMat->NumFramesDirty = gNumFrameResources;
}

void ShapesApp::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for(auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if(e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void ShapesApp::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for(auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if(mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void ShapesApp::UpdatePhysicsTimer(const GameTimer& gt)
{
	physicsTimer += gt.DeltaTime();
	if(physicsTimer > physicsFrameDuration)
	{
		physicsTimer = 0;
		if(PhysicsEngine::Instance->bodiesA.size() < 1)
		{
			std::cout << "Physics update failed, no bodies.\n";
			return;
		}
		
		PlayerGameObject* player = nullptr;
		if(gameObjects.size() > playerObjectIndex)
			player = static_cast<PlayerGameObject*>(&gameObjects[playerObjectIndex]);
		XMStoreFloat3(&PhysicsEngine::Instance->bodiesA[playerObjectIndex].forwardDirection, XMVector3Cross(XMVectorSet(0,1,0,0), XMLoadFloat3(&mRight)));
		if(player && (player->w || player->s || player->d || player->a))
		{
			float x = (player->a ? -1.0f : 0.0f) + (player->d ? 1.0f : 0.0f);
			float z = (player->s ? 1.0f : 0.0f) + (player->w ? -1.0f : 0.0f);
			XMVECTOR forwardDirection = XMLoadFloat3(&PhysicsEngine::Instance->bodiesA[playerObjectIndex].forwardDirection);
			XMVECTOR rightDirection = XMVector3Cross(forwardDirection, XMVectorSet(0,1,0,0));
			XMVECTOR moveDirection = XMVectorAdd(XMVectorScale(forwardDirection, z), XMVectorScale(rightDirection, x));

			if(abs(x) > 0 && abs(z) > 0)
			{
				moveDirection = XMVector3Normalize(moveDirection);
			}
			moveDirection = XMVectorScale(moveDirection, moveSpeed);
			// Can make this additive so other forces can be added to the player
			XMStoreFloat3(&PhysicsEngine::Instance->bodiesA[playerObjectIndex].localImpulse, moveDirection);
			player->w = false;
			player->s = false;
			player->a = false;
			player->d = false;
		}
		PhysicsEngine::Instance->PhysicsUpdate(physicsFrameDuration);
		
		// Delay camera update by a physics frame
		mCamera.SetCameraFromPhysics(mLastLook, mLastRight, mLastUp);
		mLastLook = mLook;
		mLastRight = mRight;
		mLastUp = mUp;
	}
}

void ShapesApp::UpdateWavesGPU(const GameTimer& gt)
{
	// Every quarter second, generate a random wave.
	static float t_base = 0.0f;
	if((mTimer.TotalTime() - t_base) >= 0.25f)
	{
		t_base += 0.25f;

		int i = MathHelper::Rand(4, mWaves->RowCount() - 5);
		int j = MathHelper::Rand(4, mWaves->ColumnCount() - 5);

		float r = MathHelper::RandF(1.0f, 2.0f);

		mWaves->Disturb(mCommandList.Get(), mWavesRootSignature.Get(), mPSOs["wavesDisturb"].Get(), i, j, r);
	}

	// Update the wave simulation.
	mWaves->Update(gt, mCommandList.Get(), mWavesRootSignature.Get(), mPSOs["wavesUpdate"].Get());
}

void ShapesApp::UpdateGameObjects(const GameTimer& gt)
{
	std::cout << std::to_string(gameObjects.size()) << '\n';
	for (int i = 0; i < gameObjects.size(); i++)
	{
		gameObjects[i].Update(physicsTimer, physicsFrameDuration);
	}
}

void ShapesApp::UpdateMainPassCB(const GameTimer& gt)
{
	//Camera: step 6 (use camera view and projection matrices)
	//XMMATRIX view = XMLoadFloat4x4(&mView);
	//XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));

	//Camera: step 7 (use camera pos)
	//mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.EyePosW = mCamera.GetPosition3f();
	
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	// Sun - can't tell if this is working correctly, lol
	float sunTheta = 240;
	float sunPhi = 210;
	
	float radTheta = (sunTheta / 180) * (XM_PI);
	float radPhi = (sunPhi / 180) * (XM_PI);
	float cosT = cos(radTheta);
	
	XMFLOAT3 sunDirection = {
		cos(radPhi) * cosT,
		sin(radTheta),
		sin(radPhi) * cosT
	};
	
	mMainPassCB.Lights[0].Direction = sunDirection;
	mMainPassCB.Lights[0].Strength = { 0.8f, 0.8f, 0.9f };
	
	// Add a point light to keep cupola
	mMainPassCB.Lights[1].Position = { 0.0f, 10.0f, 0.0f };
	mMainPassCB.Lights[1].Strength = { 1.0f, 0.8f, 0.0f };
	mMainPassCB.Lights[1].FalloffStart = 1;
	mMainPassCB.Lights[1].FalloffEnd = 5;

	// Add a point light to wizard tower
	mMainPassCB.Lights[2].Position = { 11, 17.5, 8 };
	mMainPassCB.Lights[2].Strength = { 1.0f, 0.0f, 0.0f };
	mMainPassCB.Lights[2].FalloffStart = 1;
	mMainPassCB.Lights[2].FalloffEnd = 5;

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void ShapesApp::BuildLabyrinth()
{
	new LabyrinthGen();
	LabyrinthGen::Instance->position = {-64, 0, -256};
	labyrinth = LabyrinthGen::Instance->Generate();
}

void ShapesApp::InitializePhysicsEngineAndGameObjects()
{
	new PhysicsEngine;
	
	gameObjects.emplace_back(PlayerGameObject(0, StartLocation));
	PhysicsEngine::Instance->AddBody(Body(StartLocation, SPHERE, {0,0,-1}, {2.0f, 0, 0}, true, 0.01));

	gameObjects.emplace_back(PlayerGameObject(1, {}));
	PhysicsEngine::Instance->AddBody(Body({0,0,0}, PLANE, {0, 1, 0}, {}, false, 0));

	for(int i = 0; i < labyrinth.size(); i++)
	{
		PhysicsEngine::Instance->AddBody(Body(labyrinth[i], AABB, {0, 1, 0}, {4,4,4}, false, 0));
	}
	
	PhysicsEngine::Instance->Init();
}

void ShapesApp::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 
        1,  // number of descriptors
        0); // register t0

	CD3DX12_DESCRIPTOR_RANGE displacementMapTable;
	displacementMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

    // Root parameter can be a table, root descriptor or root constants.
    CD3DX12_ROOT_PARAMETER slotRootParameter[5];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
    slotRootParameter[1].InitAsConstantBufferView(0); // register b0
    slotRootParameter[2].InitAsConstantBufferView(1); // register b1
    slotRootParameter[3].InitAsConstantBufferView(2); // register b2
	slotRootParameter[4].InitAsDescriptorTable(1, &displacementMapTable, D3D12_SHADER_VISIBILITY_ALL);

	auto staticSamplers = GetStaticSamplers();

    // A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(5, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
    ComPtr<ID3DBlob> serializedRootSig = nullptr;
    ComPtr<ID3DBlob> errorBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

    if(errorBlob != nullptr)
    {
        ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
    }
    ThrowIfFailed(hr);

    ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void ShapesApp::BuildWavesRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE uavTable0;
	uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE uavTable1;
	uavTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 1);

	CD3DX12_DESCRIPTOR_RANGE uavTable2;
	uavTable2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 2);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsConstants(6, 0);
	slotRootParameter[1].InitAsDescriptorTable(1, &uavTable0);
	slotRootParameter[2].InitAsDescriptorTable(1, &uavTable1);
	slotRootParameter[3].InitAsDescriptorTable(1, &uavTable2);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_NONE);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mWavesRootSignature.GetAddressOf())));
}

void ShapesApp::BuildShadersAndInputLayout()
{
	//Alpha: step 7 - adding the alpha test

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		
			//macro name, macro definition
			"ALPHA_TEST", "0",    //"1" or "0" doesn't really set anything on and off
			NULL,NULL,
	};

	const D3D_SHADER_MACRO waveDefines[] =
	{
		"DISPLACEMENT_MAP", "1",
		NULL, NULL
	};
	
	mShaders["wavesVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", waveDefines, "VS", "vs_5_1");
	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_1");
	mShaders["wavesUpdateCS"] = d3dUtil::CompileShader(L"Shaders\\WaveSim.hlsl", nullptr, "UpdateWavesCS", "cs_5_1");
	mShaders["wavesDisturbCS"] = d3dUtil::CompileShader(L"Shaders\\WaveSim.hlsl", nullptr, "DisturbWavesCS", "cs_5_1");
	
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	//Trees: Step 6.1
	mShaders["treeSpriteVS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["treeSpriteGS"] = d3dUtil::CompileShader(L"Shaders\\TreeSprite.hlsl", nullptr, "GS", "gs_5_1");
	mShaders["treeSpritePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", alphaTestDefines, "PS", "ps_5_1");

	mTreeSpriteInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void ShapesApp::BuildShapeGeometry()
{
   	GeometryGenerator geoGen;

	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 0);
	GeometryGenerator::MeshData gBox = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 0);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(10.0f, 10.0f, 40, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(2, 8, 8);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);
	GeometryGenerator::MeshData prism = geoGen.CreateTriangularPrism(1,1,1);
	GeometryGenerator::MeshData wedge = geoGen.CreateWedge(1,1,1);
	GeometryGenerator::MeshData keepCupolaWindow = geoGen.CreateTorus(2,0.25, 1, 4, 4, true);
	GeometryGenerator::MeshData keepCupolaSlope = geoGen.CreatePyramid(1, 1,1);
	GeometryGenerator::MeshData keepEdgeWall = geoGen.CreateTorus(6, 2, 0.3, 4, 4, true,3, 1);
	// broken GeometryGenerator::MeshData groundTorus = geoGen.CreateTorus(48, 1.414213, 47, 4, 4, true, 1,1, false);
	GeometryGenerator::MeshData groundTorus = geoGen.CreateTorus(48, 1.414213, 47, 4, 4, true, 20, 20);
	GeometryGenerator::MeshData wallTorus = geoGen.CreateTorus(16, 0.3, 3, 4, 4, true, 5,1);
	GeometryGenerator::MeshData pyramid = geoGen.CreatePyramid(1,1,1);
	GeometryGenerator::MeshData towerBase = geoGen.CreateCylinder(1, 0.9f, 5.0f, 20, 20);
	GeometryGenerator::MeshData towerRing = geoGen.CreateTorus(0.9, 1, 0.1,  20, 4, true, 5,2);
	GeometryGenerator::MeshData cone = geoGen.CreateCone(1,2, 20);
	GeometryGenerator::MeshData wallRing = geoGen.CreateTorus(18, 1, 0.3,  4, 4, true, 10,1);
	GeometryGenerator::MeshData spike = geoGen.CreateOctahedron(0.1, 0.1, 2, -0.5);
	GeometryGenerator::MeshData TestTor = geoGen.CreateTorus(2, 1, 1,  48, 16, false, 1,1);
	GeometryGenerator::MeshData labBlock = geoGen.CreateBox(4.0f, 4.0f, 4.0f, 0);
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.

	UINT currentVertexOffset = 0;
	UINT currentIndexOffset = 0;
	
	// Created a submesh /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	SubmeshGeometry boxSubmesh = CreateSubmesh(box, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry gBoxSubmesh = CreateSubmesh(gBox, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry gTorSubmesh = CreateSubmesh(groundTorus, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry gridSubmesh = CreateSubmesh(grid, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry sphereSubmesh = CreateSubmesh(sphere, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry cylinderSubmesh = CreateSubmesh(cylinder, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry bridgeSubmesh = CreateSubmesh(wedge, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry kCWSubmesh = CreateSubmesh(keepCupolaWindow, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry kCSSubmesh = CreateSubmesh(keepCupolaSlope, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry kEWSubmesh = CreateSubmesh(keepEdgeWall, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry wTorSubmesh = CreateSubmesh(wallTorus, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry fKRSubmesh = CreateSubmesh(prism, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry towerBaseSubmesh = CreateSubmesh(towerBase, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry towerRingSubmesh = CreateSubmesh(towerRing, currentVertexOffset, currentIndexOffset);
	SubmeshGeometry towerCapSubmesh = CreateSubmesh(cone,currentVertexOffset, currentIndexOffset);
	SubmeshGeometry wallRingSubmesh = CreateSubmesh(wallRing,currentVertexOffset, currentIndexOffset);
	SubmeshGeometry spikeSubmesh = CreateSubmesh(spike,currentVertexOffset, currentIndexOffset);
	SubmeshGeometry tTorSubmesh = CreateSubmesh(TestTor,currentVertexOffset, currentIndexOffset);
	SubmeshGeometry labBlockSubmesh = CreateSubmesh(labBlock, currentVertexOffset, currentIndexOffset);
	std::vector<Vertex> vertices(currentIndexOffset);
	std::vector<std::uint16_t> indices;
	currentVertexOffset = 0;

	// Pack vertex buffer ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AppendVerticesAndIndices(vertices, indices, box, currentVertexOffset, {1,1,0,1});
	AppendVerticesAndIndices(vertices, indices, gBox, currentVertexOffset, {0,0.6,0.1,1});
	AppendVerticesAndIndices(vertices, indices, groundTorus, currentVertexOffset, {0,0.6,0.1,1});
	AppendVerticesAndIndices(vertices, indices, grid, currentVertexOffset, {0,0.1,0.6,1});
	AppendVerticesAndIndices(vertices, indices, sphere, currentVertexOffset, {1,0,0,1});
	AppendVerticesAndIndices(vertices, indices, cylinder, currentVertexOffset, {0.5,0.5,0.6,1});
	AppendVerticesAndIndices(vertices, indices, wedge, currentVertexOffset, {1,0,0,1});
	AppendVerticesAndIndices(vertices, indices, keepCupolaWindow, currentVertexOffset, {1,0,1,1});
	AppendVerticesAndIndices(vertices, indices, keepCupolaSlope, currentVertexOffset, {0,1,1,1});
	AppendVerticesAndIndices(vertices, indices, keepEdgeWall, currentVertexOffset, {0,1,1,1});
	AppendVerticesAndIndices(vertices, indices, wallTorus, currentVertexOffset, {0,1,1,1});
	AppendVerticesAndIndices(vertices, indices, prism, currentVertexOffset, {1,0,1,1});
	AppendVerticesAndIndices(vertices, indices, towerBase, currentVertexOffset, {1,0.7,0,1});
	AppendVerticesAndIndices(vertices, indices, towerRing, currentVertexOffset, {1,0,1,1});
	AppendVerticesAndIndices(vertices, indices, cone, currentVertexOffset, {1,0.7,0,1});
	AppendVerticesAndIndices(vertices, indices, wallRing, currentVertexOffset, {1,0.7,0,1});
	AppendVerticesAndIndices(vertices, indices, spike, currentVertexOffset, {0.8,0.8,0.9,1});
	AppendVerticesAndIndices(vertices, indices, TestTor, currentVertexOffset, {0.8,0.8,0.9,1});
	AppendVerticesAndIndices(vertices, indices, labBlock, currentVertexOffset, {1,1,1,1});
	
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	// Draw call name ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["gbox"] = gBoxSubmesh;
	geo->DrawArgs["gtor"] = gTorSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;
	geo->DrawArgs["bridge"] = bridgeSubmesh;
	geo->DrawArgs["kcw"] = kCWSubmesh;
	geo->DrawArgs["kcs"] = kCSSubmesh;
	geo->DrawArgs["kew"] = kEWSubmesh;
	geo->DrawArgs["fkr"] = fKRSubmesh;
	geo->DrawArgs["wtor"] = wTorSubmesh;
	geo->DrawArgs["tbase"] = towerBaseSubmesh;
	geo->DrawArgs["tring"] = towerRingSubmesh;
	geo->DrawArgs["tcap"] = towerCapSubmesh;
	geo->DrawArgs["wring"] = wallRingSubmesh;
	geo->DrawArgs["spike"] = spikeSubmesh;
	geo->DrawArgs["ttor"] = tTorSubmesh;
	geo->DrawArgs["labBlock"] = labBlockSubmesh;
	mGeometries[geo->Name] = std::move(geo);
}

void ShapesApp::BuildWavesGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(160.0f, 160.0f, mWaves->RowCount(), mWaves->ColumnCount());

	std::vector<Vertex> vertices(grid.Vertices.size());
	for(size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		vertices[i].Pos = grid.Vertices[i].Position;
		vertices[i].Normal = grid.Vertices[i].Normal;
		vertices[i].TexC = grid.Vertices[i].TexC;
	}

	std::vector<std::uint32_t> indices = grid.Indices32;

	UINT vbByteSize = mWaves->VertexCount()*sizeof(Vertex);
	UINT ibByteSize = (UINT)indices.size()*sizeof(std::uint32_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "waterGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["waveGrid"] = submesh;

	mGeometries["waterGeo"] = std::move(geo);
}

// Cleanup submesh creation
SubmeshGeometry ShapesApp::CreateSubmesh(GeometryGenerator::MeshData mesh, UINT& curVertCount, UINT& curIndCount)
{
	SubmeshGeometry subMesh;
	subMesh.IndexCount = mesh.Indices32.size();
	
	subMesh.StartIndexLocation = curIndCount;
	curIndCount += subMesh.IndexCount;

	subMesh.BaseVertexLocation = curVertCount;
	curVertCount += mesh.Vertices.size();
	
	return subMesh;
}

void ShapesApp::AppendVerticesAndIndices(std::vector<Vertex>& vertices, std::vector<std::uint16_t>& indices, GeometryGenerator::MeshData mesh, UINT& curOffset, XMFLOAT4 color)
{
	for (size_t i = 0; i < mesh.Vertices.size(); ++i, ++curOffset)
	{
		vertices[curOffset].Pos = mesh.Vertices[i].Position;
		vertices[curOffset].Normal = mesh.Vertices[i].Normal;
		vertices[curOffset].TexC = mesh.Vertices[i].TexC;
	}
	indices.insert(indices.end(), std::begin(mesh.GetIndices16()), std::end(mesh.GetIndices16()));
}

//Trees: Step 3
void ShapesApp::BuildTreeSpritesGeometry()
{
	//Trees: Step 4 (step5 and 6 are in TreeSprite.hlsl)
	struct TreeSpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};
	static const int rTreeCount = 100;
	static const int lTreeCount = 100;
	static const int cTreeCount = 32;
	
	std::array<std::uint16_t, rTreeCount + lTreeCount + cTreeCount> indices;
	std::array<TreeSpriteVertex, rTreeCount + lTreeCount + cTreeCount> vertices;

	int index = 0;
	
	for (UINT i = 0; i < rTreeCount; ++i)
	{
		float x = MathHelper::RandF(30.0f, 100.0f);
		float z = MathHelper::RandF(-100.0f, 100.0f);
		float y = 4.0;

		vertices[index].Pos = XMFLOAT3(x, y, z);
		vertices[index].Size = XMFLOAT2(8.0f, 8.0f);

		indices[index] = index;
		index++;
	}

	for (UINT i = 0; i < cTreeCount; ++i)
	{
		float x = MathHelper::RandF(-30.0f, 30.0f);
		float z = MathHelper::RandF(30.0f, 100.0f);
		float y = 4.0;

		vertices[index].Pos = XMFLOAT3(x, y, z);
		vertices[index].Size = XMFLOAT2(8.0f, 8.0f);

		indices[index] = index;
		index++;
	}
	
	for (UINT i = 0; i < lTreeCount; ++i)
	{
		float x = MathHelper::RandF(-30.0f, -100.0f);
		float z = MathHelper::RandF(-100.0f, 100.0f);
		float y = 4.0;

		vertices[index].Pos = XMFLOAT3(x, y, z);
		vertices[index].Size = XMFLOAT2(8.0f, 8.0f);

		indices[index] = index;
		index++;
	}

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(TreeSpriteVertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "treeSpritesGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(TreeSpriteVertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	geo->DrawArgs["points"] = submesh;

	mGeometries["treeSpritesGeo"] = std::move(geo);
}

void ShapesApp::BuildPSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects.
	//
    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()), 
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	//
	// PSO for transparent objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//transparentPsoDesc.BlendState.AlphaToCoverageEnable = true;

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));

	//Alpha: step 8
	// PSO for alpha tested objects
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC alphaTestedPsoDesc = opaquePsoDesc;
	alphaTestedPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["alphaTestedPS"]->GetBufferPointer()),
		mShaders["alphaTestedPS"]->GetBufferSize()
	};
	alphaTestedPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&alphaTestedPsoDesc, IID_PPV_ARGS(&mPSOs["alphaTested"])));

	//
	// PSO for opaque wireframe objects.
	//

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&mPSOs["opaque_wireframe"])));


	// Trees: Step 8
	// PSO for tree sprites
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC treeSpritePsoDesc = opaquePsoDesc;
	treeSpritePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteVS"]->GetBufferPointer()),
		mShaders["treeSpriteVS"]->GetBufferSize()
	};
	treeSpritePsoDesc.GS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpriteGS"]->GetBufferPointer()),
		mShaders["treeSpriteGS"]->GetBufferSize()
	};
	treeSpritePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["treeSpritePS"]->GetBufferPointer()),
		mShaders["treeSpritePS"]->GetBufferSize()
	};
	
	treeSpritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	treeSpritePsoDesc.InputLayout = { mTreeSpriteInputLayout.data(), (UINT)mTreeSpriteInputLayout.size() };
	treeSpritePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&treeSpritePsoDesc, IID_PPV_ARGS(&mPSOs["treeSprites"])));

	
	//
	// PSO for drawing waves
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC wavesRenderPSO = transparentPsoDesc;
	wavesRenderPSO.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["wavesVS"]->GetBufferPointer()),
		mShaders["wavesVS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&wavesRenderPSO, IID_PPV_ARGS(&mPSOs["wavesRender"])));

	//
	// PSO for disturbing waves
	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC wavesDisturbPSO = {};
	wavesDisturbPSO.pRootSignature = mWavesRootSignature.Get();
	wavesDisturbPSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["wavesDisturbCS"]->GetBufferPointer()),
		mShaders["wavesDisturbCS"]->GetBufferSize()
	};
	wavesDisturbPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&wavesDisturbPSO, IID_PPV_ARGS(&mPSOs["wavesDisturb"])));

	//
	// PSO for updating waves
	//
	D3D12_COMPUTE_PIPELINE_STATE_DESC wavesUpdatePSO = {};
	wavesUpdatePSO.pRootSignature = mWavesRootSignature.Get();
	wavesUpdatePSO.CS =
	{
		reinterpret_cast<BYTE*>(mShaders["wavesUpdateCS"]->GetBufferPointer()),
		mShaders["wavesUpdateCS"]->GetBufferSize()
	};
	wavesUpdatePSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	ThrowIfFailed(md3dDevice->CreateComputePipelineState(&wavesUpdatePSO, IID_PPV_ARGS(&mPSOs["wavesUpdate"])));
}

void ShapesApp::BuildFrameResources()
{
    for(int i = 0; i < gNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
    }
}


void ShapesApp::LoadTextures()
{
	//Alpha: step 6 - load textures with alpha channel
	
	auto grassTex = std::make_unique<Texture>();
	grassTex->Name = "grassTex";
	grassTex->Filename = L"../../Textures/grass1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), grassTex->Filename.c_str(),
		grassTex->Resource, grassTex->UploadHeap));

	auto stone1Tex = std::make_unique<Texture>();
	stone1Tex->Name = "stone1Tex";
	stone1Tex->Filename = L"../../Textures/stone1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), stone1Tex->Filename.c_str(),
		stone1Tex->Resource, stone1Tex->UploadHeap));

	auto stone2Tex = std::make_unique<Texture>();
	stone2Tex->Name = "stone2Tex";
	stone2Tex->Filename = L"../../Textures/stone3.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), stone2Tex->Filename.c_str(),
		stone2Tex->Resource, stone2Tex->UploadHeap));

	auto waterTex = std::make_unique<Texture>();
	waterTex->Name = "waterTex";
	waterTex->Filename = L"../../Textures/water1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), waterTex->Filename.c_str(),
		waterTex->Resource, waterTex->UploadHeap));

	auto roofTex = std::make_unique<Texture>();
	roofTex->Name = "roofTex";
	roofTex->Filename = L"../../Textures/kroof.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), roofTex->Filename.c_str(),
		roofTex->Resource, roofTex->UploadHeap));

	auto woodTex = std::make_unique<Texture>();
	woodTex->Name = "woodTex";
	woodTex->Filename = L"../../Textures/wood1.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
	mCommandList.Get(), woodTex->Filename.c_str(),
	woodTex->Resource, woodTex->UploadHeap));

	//Trees: step 9
	auto treeArrayTex = std::make_unique<Texture>();
	treeArrayTex->Name = "treeArrayTex";
	treeArrayTex->Filename = L"../../Textures/treearray.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), treeArrayTex->Filename.c_str(),
		treeArrayTex->Resource, treeArrayTex->UploadHeap));

	mTextures[treeArrayTex->Name] = std::move(treeArrayTex);
	mTextures[grassTex->Name] = std::move(grassTex);
	mTextures[stone1Tex->Name] = std::move(stone1Tex);
	mTextures[stone2Tex->Name] = std::move(stone2Tex);
	mTextures[roofTex->Name] = std::move(roofTex);
	mTextures[woodTex->Name] = std::move(woodTex);
	mTextures[waterTex->Name] = std::move(waterTex);
}

void ShapesApp::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	UINT srvCount = 8;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	//Trees: step 10
	srvHeapDesc.NumDescriptors = srvCount + mWaves->DescriptorCount();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto grassTex = mTextures["grassTex"]->Resource;
	auto stone1Tex = mTextures["stone1Tex"]->Resource;
	auto stone2Tex = mTextures["stone2Tex"]->Resource;
	auto waterTex = mTextures["waterTex"]->Resource;
	auto roofTex = mTextures["roofTex"]->Resource;
	auto woodTex = mTextures["woodTex"]->Resource;
	auto steelTex = mTextures["woodTex"]->Resource;
	//Trees: step 11
	auto treeArrayTex = mTextures["treeArrayTex"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = grassTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = grassTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(grassTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = stone1Tex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = stone1Tex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(stone1Tex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = stone2Tex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = stone2Tex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(stone2Tex.Get(), &srvDesc, hDescriptor);
	
	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = waterTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = waterTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(waterTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = roofTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = roofTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(roofTex.Get(), &srvDesc, hDescriptor);

	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = woodTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = woodTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(woodTex.Get(), &srvDesc, hDescriptor);
	// next descriptor
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	srvDesc.Format = steelTex->GetDesc().Format;
	srvDesc.Texture2D.MipLevels = steelTex->GetDesc().MipLevels;
	md3dDevice->CreateShaderResourceView(steelTex.Get(), &srvDesc, hDescriptor);
	
	//Trees: step 12
	hDescriptor.Offset(1, mCbvSrvDescriptorSize);
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
	auto desc = treeArrayTex->GetDesc();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Format = treeArrayTex->GetDesc().Format;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = treeArrayTex->GetDesc().DepthOrArraySize;
	md3dDevice->CreateShaderResourceView(treeArrayTex.Get(), &srvDesc, hDescriptor);

	mWaves->BuildDescriptors(
	CD3DX12_CPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvCount, mCbvSrvDescriptorSize),
	CD3DX12_GPU_DESCRIPTOR_HANDLE(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), srvCount, mCbvSrvDescriptorSize),
	mCbvSrvDescriptorSize);
}

void ShapesApp::BuildMaterials()
{
	auto grass1 = std::make_unique<Material>();
	grass1->Name = "grass1";
	grass1->MatCBIndex = 0;
	grass1->DiffuseSrvHeapIndex = 0;
	grass1->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass1->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	grass1->Roughness = 0.3f;

	auto stone1 = std::make_unique<Material>();
	stone1->Name = "stone1";
	stone1->MatCBIndex = 1;
	stone1->DiffuseSrvHeapIndex = 1;
	stone1->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	stone1->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone1->Roughness = 0.5f;
 
	auto stone2 = std::make_unique<Material>();
	stone2->Name = "stone2";
	stone2->MatCBIndex = 2;
	stone2->DiffuseSrvHeapIndex = 2;
	stone2->DiffuseAlbedo = XMFLOAT4(0.81f, 0.83f, 1.0f, 1.0f);
	stone2->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone2->Roughness = 0.5f;
	XMStoreFloat4x4(&stone2->MatTransform, XMMatrixScaling(2,2,1));

	auto water0 = std::make_unique<Material>();
	water0->Name = "water0";
	water0->MatCBIndex = 3;
	water0->DiffuseSrvHeapIndex = 3;
	water0->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	water0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	water0->Roughness = 0.1f;

	auto roof0 = std::make_unique<Material>();
	roof0->Name = "roof0";
	roof0->MatCBIndex = 4;
	roof0->DiffuseSrvHeapIndex = 4;
	roof0->DiffuseAlbedo = XMFLOAT4(0.8f, 0.9f, 1.0f, 1.0f);
	roof0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	roof0->Roughness = 0.7f;

	auto wood0 = std::make_unique<Material>();
	wood0->Name = "wood0";
	wood0->MatCBIndex = 5;
	wood0->DiffuseSrvHeapIndex = 5;
	wood0->DiffuseAlbedo = XMFLOAT4(1.0f, 0.8f, 0.6f, 1.0f);
	wood0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	wood0->Roughness = 0.3f;

	auto steel0 = std::make_unique<Material>();
	steel0->Name = "steel0";
	steel0->MatCBIndex = 6;
	steel0->DiffuseSrvHeapIndex = 6;
	steel0->DiffuseAlbedo = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	steel0->FresnelR0 = XMFLOAT3(1.0f, 1.0f, 1.0f);
	steel0->Roughness = 0.7f;
	

	//Trees: step 11.5
	auto treeSprites = std::make_unique<Material>();
	treeSprites->Name = "treeSprites";
	treeSprites->MatCBIndex = 7;
	treeSprites->DiffuseSrvHeapIndex = 7;
	treeSprites->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	treeSprites->FresnelR0 = XMFLOAT3(0.01f, 0.01f, 0.01f);
	treeSprites->Roughness = 0.125f;

	auto grass2 = std::make_unique<Material>();
	grass2->Name = "grass2";
	grass2->MatCBIndex = 8;
	grass2->DiffuseSrvHeapIndex = 0;
	grass2->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	grass2->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	grass2->Roughness = 0.3f;
	grass2->MatTransform = {
		16,0,0,0,
		0,16,0,0,
		0,0,16,0,
		0,0,0,1
	};

	mMaterials["grass1"] = std::move(grass1);
	mMaterials["water0"] = std::move(water0);
	mMaterials["roof0"] = std::move(roof0);
	mMaterials["wood0"] = std::move(wood0);
	mMaterials["stone1"] = std::move(stone1);
	mMaterials["stone2"] = std::move(stone2);
	mMaterials["steel0"] = std::move(steel0);
	mMaterials["treeSprites"] = std::move(treeSprites);
	mMaterials["grass2"] = std::move(grass2);
	
}

void ShapesApp::BuildRenderItems()
{
	// Add items ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	UINT objCBIndex = 0;
	for (int i = 0; i < 4; ++i, objCBIndex++)
	{
		float rot = XM_PI / 2 * i;
		std::unique_ptr<RenderItem> winRitem = std::make_unique<RenderItem>();

		XMStoreFloat4x4(&winRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationX(XM_PI / 2)
			* XMMatrixTranslation(0.0f, 9, 1.75) * XMMatrixRotationY(rot));

		winRitem->ObjCBIndex = objCBIndex;
		winRitem->Geo = mGeometries["shapeGeo"].get();
		winRitem->Mat = mMaterials["stone1"].get();
		winRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		winRitem->IndexCount = winRitem->Geo->DrawArgs["kcw"].IndexCount;
		winRitem->StartIndexLocation = winRitem->Geo->DrawArgs["kcw"].StartIndexLocation;
		winRitem->BaseVertexLocation = winRitem->Geo->DrawArgs["kcw"].BaseVertexLocation;
		
		mRitemLayer[(int)RenderLayer::Opaque].push_back(winRitem.get());
		mAllRitems.push_back(std::move(winRitem));
	}

	auto boxRitem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(8.5f, 7.0f, 8.5f) * XMMatrixTranslation(0.0f, 3.5, 0.0f));

	boxRitem->ObjCBIndex = objCBIndex++;
	boxRitem->Geo = mGeometries["shapeGeo"].get();
	boxRitem->Mat = mMaterials["stone2"].get();
	boxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
	boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitem.get());
	mAllRitems.push_back(std::move(boxRitem));

	auto boxRitemBW = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&boxRitemBW->World, XMMatrixScaling(22.0f, 6.0f, 0.5f) * XMMatrixTranslation(0.0f, 3, 7.0f));

	boxRitemBW->ObjCBIndex = objCBIndex++;
	boxRitemBW->Geo = mGeometries["shapeGeo"].get();
	boxRitemBW->Mat = mMaterials["stone1"].get();
	boxRitemBW->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitemBW->IndexCount = boxRitemBW->Geo->DrawArgs["box"].IndexCount;
	boxRitemBW->StartIndexLocation = boxRitemBW->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitemBW->BaseVertexLocation = boxRitemBW->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitemBW.get());
	mAllRitems.push_back(std::move(boxRitemBW));

	auto boxRitemLW = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&boxRitemLW->World, XMMatrixScaling(0.5f, 6.0f, 22) * XMMatrixTranslation(-11, 3, -4));

	boxRitemLW->ObjCBIndex = objCBIndex++;
	boxRitemLW->Geo = mGeometries["shapeGeo"].get();
	boxRitemLW->Mat = mMaterials["stone1"].get();
	boxRitemLW->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitemLW->IndexCount = boxRitemLW->Geo->DrawArgs["box"].IndexCount;
	boxRitemLW->StartIndexLocation = boxRitemLW->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitemLW->BaseVertexLocation = boxRitemLW->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitemLW.get());
	mAllRitems.push_back(std::move(boxRitemLW));

	auto boxRitemRW = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&boxRitemRW->World, XMMatrixScaling(0.5f, 6.0f, 22) * XMMatrixTranslation(11, 3, -4));

	boxRitemRW->ObjCBIndex = objCBIndex++;
	boxRitemRW->Geo = mGeometries["shapeGeo"].get();
	boxRitemRW->Mat = mMaterials["stone1"].get();
	boxRitemRW->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitemRW->IndexCount = boxRitemRW->Geo->DrawArgs["box"].IndexCount;
	boxRitemRW->StartIndexLocation = boxRitemRW->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitemRW->BaseVertexLocation = boxRitemRW->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitemRW.get());
	mAllRitems.push_back(std::move(boxRitemRW));

	auto boxRitemRFW = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&boxRitemRFW->World, XMMatrixScaling(8.0f, 6.0f, 0.5f) * XMMatrixTranslation(6.5, 3, -15.0f));

	boxRitemRFW->ObjCBIndex = objCBIndex++;
	boxRitemRFW->Geo = mGeometries["shapeGeo"].get();
	boxRitemRFW->Mat = mMaterials["stone1"].get();
	boxRitemRFW->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitemRFW->IndexCount = boxRitemRFW->Geo->DrawArgs["box"].IndexCount;
	boxRitemRFW->StartIndexLocation = boxRitemRFW->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitemRFW->BaseVertexLocation = boxRitemRFW->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitemRFW.get());
	mAllRitems.push_back(std::move(boxRitemRFW));

	auto boxRitemLFW = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&boxRitemLFW->World, XMMatrixScaling(8.0f, 6.0f, 0.5f) * XMMatrixTranslation(-6.5, 3, -15.0f));

	boxRitemLFW->ObjCBIndex = objCBIndex++;
	boxRitemLFW->Geo = mGeometries["shapeGeo"].get();
	boxRitemLFW->Mat = mMaterials["stone1"].get();
	boxRitemLFW->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	boxRitemLFW->IndexCount = boxRitemLFW->Geo->DrawArgs["box"].IndexCount;
	boxRitemLFW->StartIndexLocation = boxRitemLFW->Geo->DrawArgs["box"].StartIndexLocation;
	boxRitemLFW->BaseVertexLocation = boxRitemLFW->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(boxRitemLFW.get());
	mAllRitems.push_back(std::move(boxRitemLFW));

	auto wTorRitem1 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&wTorRitem1->World, XMMatrixScaling(1.0f, 10.0f, 1.0f) * XMMatrixTranslation(0.0f, -1.5, -4));

	wTorRitem1->ObjCBIndex = objCBIndex++;
	wTorRitem1->Geo = mGeometries["shapeGeo"].get();
	wTorRitem1->Mat = mMaterials["stone2"].get();
	wTorRitem1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wTorRitem1->IndexCount = wTorRitem1->Geo->DrawArgs["wtor"].IndexCount;
	wTorRitem1->StartIndexLocation = wTorRitem1->Geo->DrawArgs["wtor"].StartIndexLocation;
	wTorRitem1->BaseVertexLocation = wTorRitem1->Geo->DrawArgs["wtor"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(wTorRitem1.get());
	mAllRitems.push_back(std::move(wTorRitem1));

	auto wTorRitem2 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&wTorRitem2->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 6, -4));

	wTorRitem2->ObjCBIndex = objCBIndex++;
	wTorRitem2->Geo = mGeometries["shapeGeo"].get();
	wTorRitem2->Mat = mMaterials["wood0"].get();
	wTorRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wTorRitem2->IndexCount = wTorRitem2->Geo->DrawArgs["wtor"].IndexCount;
	wTorRitem2->StartIndexLocation = wTorRitem2->Geo->DrawArgs["wtor"].StartIndexLocation;
	wTorRitem2->BaseVertexLocation = wTorRitem2->Geo->DrawArgs["wtor"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(wTorRitem2.get());
	mAllRitems.push_back(std::move(wTorRitem2));

	auto wRingRitem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&wRingRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(0.0f, 6.9, -4));

	wRingRitem->ObjCBIndex = objCBIndex++;
	wRingRitem->Geo = mGeometries["shapeGeo"].get();
	wRingRitem->Mat = mMaterials["stone1"].get();
	wRingRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wRingRitem->IndexCount = wRingRitem->Geo->DrawArgs["wring"].IndexCount;
	wRingRitem->StartIndexLocation = wRingRitem->Geo->DrawArgs["wring"].StartIndexLocation;
	wRingRitem->BaseVertexLocation = wRingRitem->Geo->DrawArgs["wring"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(wRingRitem.get());
	mAllRitems.push_back(std::move(wRingRitem));

	auto gBoxRitem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&gBoxRitem->World, XMMatrixScaling(24.0f, 12.0f, 24.0f) * XMMatrixTranslation(0.0f, -6, -4));

	gBoxRitem->ObjCBIndex = objCBIndex++;
	gBoxRitem->Geo = mGeometries["shapeGeo"].get();
	gBoxRitem->Mat = mMaterials["grass1"].get();
	gBoxRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gBoxRitem->IndexCount = gBoxRitem->Geo->DrawArgs["gbox"].IndexCount;
	gBoxRitem->StartIndexLocation = gBoxRitem->Geo->DrawArgs["gbox"].StartIndexLocation;
	gBoxRitem->BaseVertexLocation = gBoxRitem->Geo->DrawArgs["gbox"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(gBoxRitem.get());
	mAllRitems.push_back(std::move(gBoxRitem));

	auto gTorRitem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&gTorRitem->World, XMMatrixScaling(2.0f, 6, 2.0f) * XMMatrixTranslation(0.0f, -6, -4));

	gTorRitem->ObjCBIndex = objCBIndex++;
	gTorRitem->Geo = mGeometries["shapeGeo"].get();
	gTorRitem->Mat = mMaterials["grass1"].get();
	gTorRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gTorRitem->IndexCount = gTorRitem->Geo->DrawArgs["gtor"].IndexCount;
	gTorRitem->StartIndexLocation = gTorRitem->Geo->DrawArgs["gtor"].StartIndexLocation;
	gTorRitem->BaseVertexLocation = gTorRitem->Geo->DrawArgs["gtor"].BaseVertexLocation;
	
	mRitemLayer[(int)RenderLayer::Opaque].push_back(gTorRitem.get());
	mAllRitems.push_back(std::move(gTorRitem));

	auto cupolaRoof = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&cupolaRoof->World, XMMatrixScaling(6.0f, 2.0f, 6.0f) * XMMatrixTranslation(0, 11, 0));

	cupolaRoof->ObjCBIndex =  objCBIndex++;
	cupolaRoof->Geo = mGeometries["shapeGeo"].get();
	cupolaRoof->Mat = mMaterials["roof0"].get();
	cupolaRoof->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	cupolaRoof->IndexCount = cupolaRoof->Geo->DrawArgs["kcs"].IndexCount;
	cupolaRoof->StartIndexLocation = cupolaRoof->Geo->DrawArgs["kcs"].StartIndexLocation;
	cupolaRoof->BaseVertexLocation = cupolaRoof->Geo->DrawArgs["kcs"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(cupolaRoof.get());
	mAllRitems.push_back(std::move(cupolaRoof));

	auto frontKeep = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&frontKeep->World, XMMatrixScaling(6, 4, 6) * XMMatrixTranslation(0, 2, -7));

	frontKeep->ObjCBIndex =  objCBIndex++;
	frontKeep->Geo = mGeometries["shapeGeo"].get();
	frontKeep->Mat = mMaterials["stone2"].get();
	frontKeep->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	frontKeep->IndexCount = frontKeep->Geo->DrawArgs["box"].IndexCount;
	frontKeep->StartIndexLocation = frontKeep->Geo->DrawArgs["box"].StartIndexLocation;
	frontKeep->BaseVertexLocation = frontKeep->Geo->DrawArgs["box"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(frontKeep.get());
	mAllRitems.push_back(std::move(frontKeep));

	auto frontKeepRoof = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&frontKeepRoof->World, XMMatrixScaling(7, 2, 7) * XMMatrixTranslation(0, 5, -7.5));

	frontKeepRoof->ObjCBIndex =  objCBIndex++;
	frontKeepRoof->Geo = mGeometries["shapeGeo"].get();
	frontKeepRoof->Mat = mMaterials["wood0"].get();
	frontKeepRoof->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	frontKeepRoof->IndexCount = frontKeepRoof->Geo->DrawArgs["fkr"].IndexCount;
	frontKeepRoof->StartIndexLocation = frontKeepRoof->Geo->DrawArgs["fkr"].StartIndexLocation;
	frontKeepRoof->BaseVertexLocation = frontKeepRoof->Geo->DrawArgs["fkr"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(frontKeepRoof.get());
	mAllRitems.push_back(std::move(frontKeepRoof));

	auto kEWRitem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&kEWRitem->World, XMMatrixScaling(1, 0.5, 1) * XMMatrixTranslation(0, 7, 0));

	kEWRitem->ObjCBIndex =  objCBIndex++;
	kEWRitem->Geo = mGeometries["shapeGeo"].get();
	kEWRitem->Mat = mMaterials["stone1"].get();
	kEWRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	kEWRitem->IndexCount = kEWRitem->Geo->DrawArgs["kew"].IndexCount;
	kEWRitem->StartIndexLocation = kEWRitem->Geo->DrawArgs["kew"].StartIndexLocation;
	kEWRitem->BaseVertexLocation = kEWRitem->Geo->DrawArgs["kew"].BaseVertexLocation;
	
	mRitemLayer[(int)RenderLayer::Opaque].push_back(kEWRitem.get());
	mAllRitems.push_back(std::move(kEWRitem));

	//auto waterRitem = std::make_unique<RenderItem>();
	//
	//XMStoreFloat4x4(&waterRitem->World, XMMatrixScaling(6, 1, 6) * XMMatrixTranslation(0, -3, 0));
	//waterRitem->ObjCBIndex =  objCBIndex++;
	//waterRitem->Geo = mGeometries["shapeGeo"].get();
	//waterRitem->Mat = mMaterials["water0"].get();
	//waterRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//waterRitem->IndexCount = waterRitem->Geo->DrawArgs["grid"].IndexCount;
	//waterRitem->StartIndexLocation = waterRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	//waterRitem->BaseVertexLocation = waterRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
	//
	//mRitemLayer[(int)RenderLayer::Transparent].push_back(waterRitem.get());
	//mAllRitems.push_back(std::move(waterRitem));

	auto wavesRitem = std::make_unique<RenderItem>();
	XMStoreFloat4x4(&wavesRitem->World, XMMatrixScaling(0.5, 0.5, 0.5) * XMMatrixTranslation(0, -3, 0));
	XMStoreFloat4x4(&wavesRitem->TexTransform, XMMatrixScaling(5.0f, 5.0f, 1.0f));
	wavesRitem->DisplacementMapTexelSize.x = 1.0f / mWaves->ColumnCount();
	wavesRitem->DisplacementMapTexelSize.y = 1.0f / mWaves->RowCount();
	wavesRitem->GridSpatialStep = mWaves->SpatialStep();
	wavesRitem->ObjCBIndex = objCBIndex++;
	wavesRitem->Mat = mMaterials["water0"].get();
	wavesRitem->Geo = mGeometries["waterGeo"].get();
	wavesRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	wavesRitem->IndexCount = wavesRitem->Geo->DrawArgs["waveGrid"].IndexCount;
	wavesRitem->StartIndexLocation = wavesRitem->Geo->DrawArgs["waveGrid"].StartIndexLocation;
	wavesRitem->BaseVertexLocation = wavesRitem->Geo->DrawArgs["waveGrid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::GpuWaves].push_back(wavesRitem.get());
	mAllRitems.push_back(std::move(wavesRitem));

	auto gUnderwaterRitem = std::make_unique<RenderItem>();
	
	XMStoreFloat4x4(&gUnderwaterRitem->World, XMMatrixScaling(6, 1, 6) * XMMatrixTranslation(0, -7, 0));
	gUnderwaterRitem->ObjCBIndex =  objCBIndex++;
	gUnderwaterRitem->Geo = mGeometries["shapeGeo"].get();
	gUnderwaterRitem->Mat = mMaterials["grass1"].get();
	gUnderwaterRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	gUnderwaterRitem->IndexCount = gUnderwaterRitem->Geo->DrawArgs["grid"].IndexCount;
	gUnderwaterRitem->StartIndexLocation = gUnderwaterRitem->Geo->DrawArgs["grid"].StartIndexLocation;
	gUnderwaterRitem->BaseVertexLocation = gUnderwaterRitem->Geo->DrawArgs["grid"].BaseVertexLocation;
	
	mRitemLayer[(int)RenderLayer::Opaque].push_back(gUnderwaterRitem.get());
	mAllRitems.push_back(std::move(gUnderwaterRitem));

	auto mazeGrass = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&mazeGrass->World, XMMatrixScaling(20, 1, 20) * XMMatrixTranslation(0, 0.01, -200));
	mazeGrass->ObjCBIndex =  objCBIndex++;
	mazeGrass->Geo = mGeometries["shapeGeo"].get();
	mazeGrass->Mat = mMaterials["grass2"].get();
	mazeGrass->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mazeGrass->IndexCount = mazeGrass->Geo->DrawArgs["grid"].IndexCount;
	mazeGrass->StartIndexLocation = mazeGrass->Geo->DrawArgs["grid"].StartIndexLocation;
	mazeGrass->BaseVertexLocation = mazeGrass->Geo->DrawArgs["grid"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(mazeGrass.get());
	mAllRitems.push_back(std::move(mazeGrass));

	auto bridgeRitem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&bridgeRitem->World, XMMatrixScaling(10, 4, 4) * XMMatrixRotationX(XM_PI) * XMMatrixRotationY(XM_PI / 2) * XMMatrixTranslation(0, -2, -20));
	bridgeRitem->ObjCBIndex =  objCBIndex++;
	bridgeRitem->Geo = mGeometries["shapeGeo"].get();
	bridgeRitem->Mat = mMaterials["stone1"].get();
	bridgeRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	bridgeRitem->IndexCount = bridgeRitem->Geo->DrawArgs["bridge"].IndexCount;
	bridgeRitem->StartIndexLocation = bridgeRitem->Geo->DrawArgs["bridge"].StartIndexLocation;
	bridgeRitem->BaseVertexLocation = bridgeRitem->Geo->DrawArgs["bridge"].BaseVertexLocation;
	
	mRitemLayer[(int)RenderLayer::Opaque].push_back(bridgeRitem.get());
	mAllRitems.push_back(std::move(bridgeRitem));

	auto tBaseRitem1 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tBaseRitem1->World, XMMatrixScaling(3, 4, 3) * XMMatrixTranslation(11, 0, -16));
	tBaseRitem1->ObjCBIndex =  objCBIndex++;
	tBaseRitem1->Geo = mGeometries["shapeGeo"].get();
	tBaseRitem1->Mat = mMaterials["stone2"].get();
	tBaseRitem1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tBaseRitem1->IndexCount = tBaseRitem1->Geo->DrawArgs["tbase"].IndexCount;
	tBaseRitem1->StartIndexLocation = tBaseRitem1->Geo->DrawArgs["tbase"].StartIndexLocation;
	tBaseRitem1->BaseVertexLocation = tBaseRitem1->Geo->DrawArgs["tbase"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tBaseRitem1.get());
	mAllRitems.push_back(std::move(tBaseRitem1));

	auto tBaseRitem2 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tBaseRitem2->World, XMMatrixScaling(3, 4, 3) * XMMatrixTranslation(-11, 0, -16));
	tBaseRitem2->ObjCBIndex =  objCBIndex++;
	tBaseRitem2->Geo = mGeometries["shapeGeo"].get();
	tBaseRitem2->Mat = mMaterials["stone2"].get();
	tBaseRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tBaseRitem2->IndexCount = tBaseRitem2->Geo->DrawArgs["tbase"].IndexCount;
	tBaseRitem2->StartIndexLocation = tBaseRitem2->Geo->DrawArgs["tbase"].StartIndexLocation;
	tBaseRitem2->BaseVertexLocation = tBaseRitem2->Geo->DrawArgs["tbase"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tBaseRitem2.get());
	mAllRitems.push_back(std::move(tBaseRitem2));

	auto tBaseRitem3 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tBaseRitem3->World, XMMatrixScaling(3, 4, 3) * XMMatrixTranslation(-11, 0, 8));
	tBaseRitem3->ObjCBIndex =  objCBIndex++;
	tBaseRitem3->Geo = mGeometries["shapeGeo"].get();
	tBaseRitem3->Mat = mMaterials["stone2"].get();
	tBaseRitem3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tBaseRitem3->IndexCount = tBaseRitem3->Geo->DrawArgs["tbase"].IndexCount;
	tBaseRitem3->StartIndexLocation = tBaseRitem3->Geo->DrawArgs["tbase"].StartIndexLocation;
	tBaseRitem3->BaseVertexLocation = tBaseRitem3->Geo->DrawArgs["tbase"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tBaseRitem3.get());
	mAllRitems.push_back(std::move(tBaseRitem3));

	auto tBaseRitem4 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tBaseRitem4->World, XMMatrixScaling(4, 5, 4) * XMMatrixTranslation(11, 2, 8));
	tBaseRitem4->ObjCBIndex =  objCBIndex++;
	tBaseRitem4->Geo = mGeometries["shapeGeo"].get();
	tBaseRitem4->Mat = mMaterials["stone2"].get();
	tBaseRitem4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tBaseRitem4->IndexCount = tBaseRitem4->Geo->DrawArgs["tbase"].IndexCount;
	tBaseRitem4->StartIndexLocation = tBaseRitem4->Geo->DrawArgs["tbase"].StartIndexLocation;
	tBaseRitem4->BaseVertexLocation = tBaseRitem4->Geo->DrawArgs["tbase"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tBaseRitem4.get());
	mAllRitems.push_back(std::move(tBaseRitem4));

	auto tRingRitem1 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tRingRitem1->World, XMMatrixScaling(3, 2, 3) * XMMatrixTranslation(11, 9, -16));
	tRingRitem1->ObjCBIndex =  objCBIndex++;
	tRingRitem1->Geo = mGeometries["shapeGeo"].get();
	tRingRitem1->Mat = mMaterials["stone1"].get();
	tRingRitem1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tRingRitem1->IndexCount = tRingRitem1->Geo->DrawArgs["tring"].IndexCount;
	tRingRitem1->StartIndexLocation = tRingRitem1->Geo->DrawArgs["tring"].StartIndexLocation;
	tRingRitem1->BaseVertexLocation = tRingRitem1->Geo->DrawArgs["tring"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tRingRitem1.get());
	mAllRitems.push_back(std::move(tRingRitem1));

	auto tRingRitem2 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tRingRitem2->World, XMMatrixScaling(3, 2, 3) * XMMatrixTranslation(-11, 9, -16));
	tRingRitem2->ObjCBIndex =  objCBIndex++;
	tRingRitem2->Geo = mGeometries["shapeGeo"].get();
	tRingRitem2->Mat = mMaterials["stone1"].get();
	tRingRitem2->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tRingRitem2->IndexCount = tRingRitem2->Geo->DrawArgs["tring"].IndexCount;
	tRingRitem2->StartIndexLocation = tRingRitem2->Geo->DrawArgs["tring"].StartIndexLocation;
	tRingRitem2->BaseVertexLocation = tRingRitem2->Geo->DrawArgs["tring"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tRingRitem2.get());
	mAllRitems.push_back(std::move(tRingRitem2));

	auto tRingRitem3 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tRingRitem3->World, XMMatrixScaling(4, 2, 4) * XMMatrixTranslation(11, 14, 8));
	tRingRitem3->ObjCBIndex =  objCBIndex++;
	tRingRitem3->Geo = mGeometries["shapeGeo"].get();
	tRingRitem3->Mat = mMaterials["stone1"].get();
	tRingRitem3->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tRingRitem3->IndexCount = tRingRitem3->Geo->DrawArgs["tring"].IndexCount;
	tRingRitem3->StartIndexLocation = tRingRitem3->Geo->DrawArgs["tring"].StartIndexLocation;
	tRingRitem3->BaseVertexLocation = tRingRitem3->Geo->DrawArgs["tring"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tRingRitem3.get());
	mAllRitems.push_back(std::move(tRingRitem3));

	auto tRingRitem4 = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tRingRitem4->World, XMMatrixScaling(3, 2, 3) * XMMatrixTranslation(-11, 9, 8));
	tRingRitem4->ObjCBIndex =  objCBIndex++;
	tRingRitem4->Geo = mGeometries["shapeGeo"].get();
	tRingRitem4->Mat = mMaterials["stone1"].get();
	tRingRitem4->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tRingRitem4->IndexCount = tRingRitem4->Geo->DrawArgs["tring"].IndexCount;
	tRingRitem4->StartIndexLocation = tRingRitem4->Geo->DrawArgs["tring"].StartIndexLocation;
	tRingRitem4->BaseVertexLocation = tRingRitem4->Geo->DrawArgs["tring"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tRingRitem4.get());
	mAllRitems.push_back(std::move(tRingRitem4));

	for (int i = 0; i < 8; ++i)
	{
		auto columnRitem = std::make_unique<RenderItem>();
		XMMATRIX colWorld = XMMatrixScaling(0.5,1,0.5) * XMMatrixTranslation(0,0,2) * XMMatrixRotationY(2 * XM_PI / 8 * i) * XMMatrixTranslation(11, 16, 8);

		XMStoreFloat4x4(&columnRitem->World, colWorld);

		columnRitem->ObjCBIndex = objCBIndex++;
		columnRitem->Geo = mGeometries["shapeGeo"].get();
		columnRitem->Mat = mMaterials["stone1"].get();
		columnRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		columnRitem->IndexCount = columnRitem->Geo->DrawArgs["cylinder"].IndexCount;
		columnRitem->StartIndexLocation = columnRitem->Geo->DrawArgs["cylinder"].StartIndexLocation;
		columnRitem->BaseVertexLocation = columnRitem->Geo->DrawArgs["cylinder"].BaseVertexLocation;

		mRitemLayer[(int)RenderLayer::Opaque].push_back(columnRitem.get());
		mAllRitems.push_back(std::move(columnRitem));
	}

	for (int i = 0; i < 5; ++i)
	{
		auto spikeRitem = std::make_unique<RenderItem>();
		XMMATRIX colWorld = XMMatrixScaling(3,2,3) * XMMatrixTranslation(-2 + i * 1, 1.75, -15);

		XMStoreFloat4x4(&spikeRitem->World, colWorld);

		spikeRitem->ObjCBIndex = objCBIndex++;
		spikeRitem->Geo = mGeometries["shapeGeo"].get();
		spikeRitem->Mat = mMaterials["steel0"].get();
		spikeRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		spikeRitem->IndexCount = spikeRitem->Geo->DrawArgs["spike"].IndexCount;
		spikeRitem->StartIndexLocation = spikeRitem->Geo->DrawArgs["spike"].StartIndexLocation;
		spikeRitem->BaseVertexLocation = spikeRitem->Geo->DrawArgs["spike"].BaseVertexLocation;

		mRitemLayer[(int)RenderLayer::Opaque].push_back(spikeRitem.get());
		mAllRitems.push_back(std::move(spikeRitem));
	}

	auto tCapRitem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tCapRitem->World, XMMatrixScaling(4, 2, 4) * XMMatrixTranslation(11, 17.5, 8));
	tCapRitem->ObjCBIndex =  objCBIndex++;
	tCapRitem->Geo = mGeometries["shapeGeo"].get();
	tCapRitem->Mat = mMaterials["roof0"].get();
	tCapRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tCapRitem->IndexCount = tCapRitem->Geo->DrawArgs["tcap"].IndexCount;
	tCapRitem->StartIndexLocation = tCapRitem->Geo->DrawArgs["tcap"].StartIndexLocation;
	tCapRitem->BaseVertexLocation = tCapRitem->Geo->DrawArgs["tcap"].BaseVertexLocation;
	
	mRitemLayer[(int)RenderLayer::Opaque].push_back(tCapRitem.get());
	mAllRitems.push_back(std::move(tCapRitem));

	//test torus
	auto tTorRitem = std::make_unique<RenderItem>();

	XMStoreFloat4x4(&tTorRitem->World, XMMatrixRotationX(XM_PI / 2) * XMMatrixScaling(1, 1, 1) * XMMatrixTranslation(11, 28, 8));
	tTorRitem->ObjCBIndex =  objCBIndex++;
	tTorRitem->Geo = mGeometries["shapeGeo"].get();
	tTorRitem->Mat = mMaterials["stone1"].get();
	tTorRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	tTorRitem->IndexCount = tTorRitem->Geo->DrawArgs["ttor"].IndexCount;
	tTorRitem->StartIndexLocation = tTorRitem->Geo->DrawArgs["ttor"].StartIndexLocation;
	tTorRitem->BaseVertexLocation = tTorRitem->Geo->DrawArgs["ttor"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::Opaque].push_back(tTorRitem.get());
	mAllRitems.push_back(std::move(tTorRitem));
	
	//Trees: step13
	auto treeSpritesRitem = std::make_unique<RenderItem>();
	treeSpritesRitem->World = MathHelper::Identity4x4();
	treeSpritesRitem->ObjCBIndex = objCBIndex++;
	treeSpritesRitem->Mat = mMaterials["treeSprites"].get();
	treeSpritesRitem->Geo = mGeometries["treeSpritesGeo"].get();
	treeSpritesRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	treeSpritesRitem->IndexCount = treeSpritesRitem->Geo->DrawArgs["points"].IndexCount;
	treeSpritesRitem->StartIndexLocation = treeSpritesRitem->Geo->DrawArgs["points"].StartIndexLocation;
	treeSpritesRitem->BaseVertexLocation = treeSpritesRitem->Geo->DrawArgs["points"].BaseVertexLocation;

	mRitemLayer[(int)RenderLayer::AlphaTestedTreeSprites].push_back(treeSpritesRitem.get());
	mAllRitems.push_back(std::move(treeSpritesRitem));
	
	for(int i = 0; i < labyrinth.size(); i++)
	{
		auto labBlockRitem = std::make_unique<RenderItem>();
		XMStoreFloat4x4(&labBlockRitem->World, XMMatrixScaling(1, 1, 1) * XMMatrixTranslation(labyrinth[i].x, labyrinth[i].y, labyrinth[i].z));
		labBlockRitem->ObjCBIndex = objCBIndex++;
		labBlockRitem->Geo = mGeometries["shapeGeo"].get();
		labBlockRitem->Mat = mMaterials["stone1"].get();
		labBlockRitem->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		labBlockRitem->IndexCount = labBlockRitem->Geo->DrawArgs["labBlock"].IndexCount;
		labBlockRitem->StartIndexLocation = labBlockRitem->Geo->DrawArgs["labBlock"].StartIndexLocation;
		labBlockRitem->BaseVertexLocation = labBlockRitem->Geo->DrawArgs["labBlock"].BaseVertexLocation;

		mRitemLayer[(int)RenderLayer::Opaque].push_back(labBlockRitem.get());
		mAllRitems.push_back(std::move(labBlockRitem));
	}
	
}

void ShapesApp::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
 
	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

    // For each render item...
    for(size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];

        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex*objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex*matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
        cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
        cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> ShapesApp::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return { 
		pointWrap, pointClamp,
		linearWrap, linearClamp, 
		anisotropicWrap, anisotropicClamp };
}

