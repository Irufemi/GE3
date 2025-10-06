#include "DrawManager.h"

#include<Windows.h>
#include <cassert>

#include <dxgidebug.h>
#include "../3D/SphereClass.h"
#include "../2D/Sprite.h"
#include "../3D/ObjClass.h"
#include "../3D/TriangleClass.h"
#include "../3D/ParticleClass.h"

#include "../source/D3D12ResourceUtil.h"

void DrawManager::Initialize(
    ID3D12GraphicsCommandList* commandList,
    ID3D12CommandQueue* commandQueue,
    IDXGISwapChain4* swapChain,
    ID3D12Fence* fence,
    HANDLE& fenceEvent,
    ID3D12CommandAllocator* commandAllocator,
    ID3D12DescriptorHeap* srvDescriptorHeap,
    ID3D12RootSignature* rootSignature
) {
    commandList_ = commandList;
    commandQueue_ = commandQueue;
    swapChain_ = swapChain;
    fence_ = fence;
    fenceEvent_ = fenceEvent;
    commandAllocator_ = commandAllocator;
    srvDescriptorHeap_ = srvDescriptorHeap;
    rootSignature_ = rootSignature;
}

void DrawManager::Finalize() {
    if (commandList_) { commandList_ = nullptr; }
    if (commandQueue_) { commandQueue_ = nullptr; }
    if (swapChain_) { swapChain_ = nullptr; }
    if (fence_) { fence_ = nullptr; }
    if (fenceEvent_) { fenceEvent_ = nullptr; }
    if (commandAllocator_) { commandAllocator_ = nullptr; }
    if (srvDescriptorHeap_) { srvDescriptorHeap_ = nullptr; }
    if (rootSignature_) { rootSignature_ = nullptr; }
}

void DrawManager::BindPSO(ID3D12PipelineState* pso) {
    if (!pso) { return; }
    commandList_->SetPipelineState(pso);
}

void DrawManager::PreDraw(
    ID3D12Resource* backBufferResource,
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
    ID3D12DescriptorHeap* dsvDescriptorHeap,
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,
    std::array<float, 4> clearColor,
    float clearDepth,
    uint8_t clearStencil
) {
    /*完璧な画面クリアを目指して*/

    ///TransitionBarrierを張るコード

    //TransitionBarrierの設定
    D3D12_RESOURCE_BARRIER barrier{};
    //今回のバリアはTransition
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    //Noneにしておく
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    //バリアを張る対象のリソース。現在のバックバッファに対して行う
    barrier.Transition.pResource = backBufferResource;
    //遷移前(現在)のResourceState
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    //遷移後のResourceState
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    //TransitionBarrierを張る
    commandList_->ResourceBarrier(1, &barrier);

    /*画面の色を変えよう*/

    ///コマンドを積み込んで確定させる

    //描画先のRTVを設定する
    commandList_->OMSetRenderTargets(1, &rtvHandle, false, nullptr);
    //指定した色で画面全体をクリアする
    commandList_->ClearRenderTargetView(rtvHandle, clearColor.data(), 0, nullptr);

    /*前後関係を正しくしよう*/

    ///DSVを設定する

    //描画先のRTVとDSVを設定する
    dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    commandList_->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

    //指定した深度で画面全体をクリアする
    commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, clearDepth, clearStencil, 0, nullptr);

    /*開発のUIを出そう*/

    ///ImGuiを描画する

    //描画用のDescriptorHeapの設定
    ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap_ };
    commandList_->SetDescriptorHeaps(1, descriptorHeaps);
}

void DrawManager::PostDraw(
    ID3D12Resource* backBufferResource,
    uint64_t& fenceValue
) {

    D3D12_RESOURCE_BARRIER barrier{};

    /*完璧な画面クリアを目指して*/

    //今回のバリアはTransition
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    //Noneにしておく
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    //バリアを張る対象のリソース。現在のバックバッファに対して行う
    barrier.Transition.pResource = backBufferResource;
    //遷移前(現在)のResourceState
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    //遷移後のResourceState
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    //画面に描く処理はすべて終わり、画面に映すので、状態を遷移
    //今回はRenderTargetからPresentにする
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    //TransitionBarrierを張る                                                                                                                                                                                                                                                                                                                                                                                                                                                                rrierを張る
    commandList_->ResourceBarrier(1, &barrier);

    /*画面の色を変えよう*/

    ///コマンドを積み込んで確定させる

    //コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
    HRESULT hr = commandList_->Close();
    assert(SUCCEEDED(hr));

    ///コマンドをキックする

    //GPUにコマンドリストの実行を行わせる
    ID3D12CommandList* commandLists[] = { commandList_ };
    commandQueue_->ExecuteCommandLists(1, commandLists);
    //GPUとOSに画面の交換を行うよう通知する
    swapChain_->Present(1, 0);

    /*完璧な画面クリアを目指して*/

    ///GPUにSignal(シグナル)を送る

    //Fenceの値を更新
    fenceValue++;
    //GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
    commandQueue_->Signal(fence_, fenceValue);

    ///Fenceの値を確認してGPUを待つ

    //Fenceの値が指定したSignal値にたどり着いているか確認する
    //GetCompletedValueの初期値はFence作成時に渡した初期値
    if (fence_->GetCompletedValue() < fenceValue) {
        //指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
        fence_->SetEventOnCompletion(fenceValue, fenceEvent_);
        //イベント待つ
        WaitForSingleObject(fenceEvent_, INFINITE);
    }

    /*画面の色を変えよう*/

    ///コマンドを積み込んで確定させる

    //次のフレーム用のコマンドリストを準備
    hr = commandAllocator_->Reset();
    assert(SUCCEEDED(hr));
    hr = commandList_->Reset(commandAllocator_, nullptr);
    assert(SUCCEEDED(hr));

}

void DrawManager::DrawTriangle(
    D3D12_VIEWPORT& viewport,
    D3D12_RECT& scissorRect,
    D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
    ID3D12Resource* materialResource,
    ID3D12Resource* wvpResource,
    ID3D12Resource* directionalLightResource,
    D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU
) {

    /*三角形を表示しよう*/
    commandList_->RSSetViewports(1, &viewport); //viewportを設定
    commandList_->RSSetScissorRects(1, &scissorRect); //Scirssorを設定
    //RootSignatureを設定。PSOに設定しているけど別途指定が必要
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
    //形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    /*三角形の色を変えよう*/

    ///CBVを設定する

    //マテリアルCBufferの場所を設定(ここでの第一引数の0はRootParameter配列の0番目であり、registerの0ではない)
    commandList_->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

    /*三角形を動かそう*/

    //wvp用のCbufferの場所を設定(今回はRootParameter[1]に対してCBVの設定を行っている)
    commandList_->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());


    commandList_->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());

    /*テクスチャを貼ろう*/

    ///DescriptorTableを設定する

    //SRVのDescriptorTableの先頭を設定。2はRootParameter[2]である。
    commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

    /*三角形を表示しよう*/

    //描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList_->DrawInstanced(3, 1, 0, 0);

}

void DrawManager::DrawSprite(
    D3D12_VIEWPORT& viewport,
    D3D12_RECT& scissorRect,
    Sprite* sprite
) {

    // 1. ビューポートとシザー矩形の設定

    /*三角形を表示しよう*/
    commandList_->RSSetViewports(1, &viewport); //viewportを設定
    commandList_->RSSetScissorRects(1, &scissorRect); //Scirssorを設定

    // 2. パイプラインの基本構成（RootSignature, PSO）

    //RootSignatureを設定。PSOに設定しているけど別途指定が必要
    commandList_->SetGraphicsRootSignature(rootSignature_);

    // 3. バッファ設定（VBV、IBV、Topology）

    //形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //VBVを設定
    commandList_->IASetVertexBuffers(0, 1, &sprite->GetD3D12Resource()->vertexBufferView_);
    //IBVを設定
    commandList_->IASetIndexBuffer(&sprite->GetD3D12Resource()->indexBufferView_);

    // 4. 定数バッファ（CBV）やライト用CBVの設定

    ///CBVを設定する

    //マテリアルCBufferの場所を設定(ここでの第一引数の0はRootParameter配列の0番目であり、registerの0ではない)
    commandList_->SetGraphicsRootConstantBufferView(0, sprite->GetD3D12Resource()->materialResource_->GetGPUVirtualAddress());

    //wvp用のCbufferの場所を設定(今回はRootParameter[1]に対してCBVの設定を行っている)
    commandList_->SetGraphicsRootConstantBufferView(1, sprite->GetD3D12Resource()->transformationResource_->GetGPUVirtualAddress());

    commandList_->SetGraphicsRootConstantBufferView(3, sprite->GetD3D12Resource()->directionalLightResource_->GetGPUVirtualAddress());

    // 5. テクスチャ用のDescriptor Table設定（SRV）

    /*テクスチャを貼ろう*/

    //SRVのDescriptorTableの先頭を設定。2はRootParameter[2]である。
    commandList_->SetGraphicsRootDescriptorTable(2, sprite->GetD3D12Resource()->textureHandle_);

    // 6. 描画

    /*三角形を表示しよう*/

    //描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList_->DrawIndexedInstanced(static_cast<UINT>(sprite->GetD3D12Resource()->indexDataList_.size()), 1, 0, 0, 0);

}

void DrawManager::DrawSphere(
    D3D12_VIEWPORT& viewport,
    D3D12_RECT& scissorRect,
    SphereClass* sphere
) {

    /*三角形を表示しよう*/
    commandList_->RSSetViewports(1, &viewport); //viewportを設定
    commandList_->RSSetScissorRects(1, &scissorRect); //Scirssorを設定
    //RootSignatureを設定。PSOに設定しているけど別途指定が必要
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->IASetVertexBuffers(0, 1, &sphere->GetD3D12Resource()->vertexBufferView_); // VBVを設定
    //IBVを設定
    commandList_->IASetIndexBuffer(&sphere->GetD3D12Resource()->indexBufferView_);
    //形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    /*三角形の色を変えよう*/

    ///CBVを設定する

    //マテリアルCBufferの場所を設定(ここでの第一引数の0はRootParameter配列の0番目であり、registerの0ではない)
    commandList_->SetGraphicsRootConstantBufferView(0, sphere->GetD3D12Resource()->materialResource_->GetGPUVirtualAddress());

    /*三角形を動かそう*/

    //wvp用のCbufferの場所を設定(今回はRootParameter[1]に対してCBVの設定を行っている)
    commandList_->SetGraphicsRootConstantBufferView(1, sphere->GetD3D12Resource()->transformationResource_->GetGPUVirtualAddress());

    commandList_->SetGraphicsRootConstantBufferView(3, sphere->GetD3D12Resource()->directionalLightResource_->GetGPUVirtualAddress());

    /*テクスチャを貼ろう*/

    ///DescriptorTableを設定する

    //SRVのDescriptorTableの先頭を設定。2はRootParameter[2]である。
    commandList_->SetGraphicsRootDescriptorTable(2, sphere->GetD3D12Resource()->textureHandle_);

    /*三角形を表示しよう*/

    //描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList_->DrawIndexedInstanced(static_cast<UINT>(sphere->GetD3D12Resource()->indexDataList_.size()), 1, 0, 0, 0);

}

void DrawManager::DrawParticle(
    D3D12_VIEWPORT& viewport,
    D3D12_RECT& scissorRect,
    ParticleClass* resource
) {

    /*三角形を表示しよう*/
    commandList_->RSSetViewports(1, &viewport); //viewportを設定
    commandList_->RSSetScissorRects(1, &scissorRect); //Scirssorを設定
    //RootSignatureを設定。PSOに設定しているけど別途指定が必要
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->IASetVertexBuffers(0, 1, &resource->GetD3D12Resource()->vertexBufferView_); // VBVを設定
    //IBVを設定
    commandList_->IASetIndexBuffer(&resource->GetD3D12Resource()->indexBufferView_);
    //形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    /*三角形の色を変えよう*/

    ///CBVを設定する

    //マテリアルCBufferの場所を設定(ここでの第一引数の0はRootParameter配列の0番目であり、registerの0ではない)
    commandList_->SetGraphicsRootConstantBufferView(0, resource->GetD3D12Resource()->materialResource_->GetGPUVirtualAddress());

    auto instancing = resource->GetInstancingSrvHandleGPU();
    assert(instancing.ptr != 0 && "Instancing SRV handle is null or invalid");
    commandList_->SetGraphicsRootDescriptorTable(4, resource->GetInstancingSrvHandleGPU());

    /*テクスチャを貼ろう*/

    ///DescriptorTableを設定する

    //SRVのDescriptorTableの先頭を設定。2はRootParameter[2]である。
    commandList_->SetGraphicsRootDescriptorTable(2, resource->GetD3D12Resource()->textureHandle_);

    /*三角形を表示しよう*/

    //描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList_->DrawIndexedInstanced(static_cast<UINT>(resource->GetD3D12Resource()->indexDataList_.size()), resource->GetInstanceCount(), 0, 0, 0);

}

void DrawManager::DrawByIndex(
    D3D12_VIEWPORT& viewport,
    D3D12_RECT& scissorRect,
    D3D12ResourceUtil* resource
) {

    /*三角形を表示しよう*/
    commandList_->RSSetViewports(1, &viewport); //viewportを設定
    commandList_->RSSetScissorRects(1, &scissorRect); //Scirssorを設定
    //RootSignatureを設定。PSOに設定しているけど別途指定が必要
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->IASetVertexBuffers(0, 1, &resource->vertexBufferView_); // VBVを設定
    //IBVを設定
    commandList_->IASetIndexBuffer(&resource->indexBufferView_);
    //形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    /*三角形の色を変えよう*/

    ///CBVを設定する

    //マテリアルCBufferの場所を設定(ここでの第一引数の0はRootParameter配列の0番目であり、registerの0ではない)
    commandList_->SetGraphicsRootConstantBufferView(0, resource->materialResource_->GetGPUVirtualAddress());

    /*三角形を動かそう*/

    //wvp用のCbufferの場所を設定(今回はRootParameter[1]に対してCBVの設定を行っている)
    commandList_->SetGraphicsRootConstantBufferView(1, resource->transformationResource_->GetGPUVirtualAddress());

    commandList_->SetGraphicsRootConstantBufferView(3, resource->directionalLightResource_->GetGPUVirtualAddress());

    /*テクスチャを貼ろう*/

    ///DescriptorTableを設定する

    //SRVのDescriptorTableの先頭を設定。2はRootParameter[2]である。
    commandList_->SetGraphicsRootDescriptorTable(2, resource->textureHandle_);

    /*三角形を表示しよう*/

    //描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList_->DrawIndexedInstanced(static_cast<UINT>(resource->indexDataList_.size()), 1, 0, 0, 0);

}

void DrawManager::DrawByVertex(
    D3D12_VIEWPORT& viewport,
    D3D12_RECT& scissorRect,
    D3D12ResourceUtil* resource
) {

    /*三角形を表示しよう*/
    commandList_->RSSetViewports(1, &viewport); //viewportを設定
    commandList_->RSSetScissorRects(1, &scissorRect); //Scirssorを設定
    //RootSignatureを設定。PSOに設定しているけど別途指定が必要
    commandList_->SetGraphicsRootSignature(rootSignature_);
    commandList_->IASetVertexBuffers(0, 1, &resource->vertexBufferView_); // VBVを設定
    //形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
    commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    /*三角形の色を変えよう*/

    ///CBVを設定する

    //マテリアルCBufferの場所を設定(ここでの第一引数の0はRootParameter配列の0番目であり、registerの0ではない)
    commandList_->SetGraphicsRootConstantBufferView(0, resource->materialResource_->GetGPUVirtualAddress());

    /*三角形を動かそう*/

    //wvp用のCbufferの場所を設定(今回はRootParameter[1]に対してCBVの設定を行っている)
    commandList_->SetGraphicsRootConstantBufferView(1, resource->transformationResource_->GetGPUVirtualAddress());


    commandList_->SetGraphicsRootConstantBufferView(3, resource->directionalLightResource_->GetGPUVirtualAddress());

    /*テクスチャを貼ろう*/

    ///DescriptorTableを設定する

    //SRVのDescriptorTableの先頭を設定。2はRootParameter[2]である。
    commandList_->SetGraphicsRootDescriptorTable(2, resource->textureHandle_);

    /*三角形を表示しよう*/

    //描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。インスタンスについては今後
    commandList_->DrawInstanced(static_cast<UINT>(resource->vertexDataList_.size()), 1, 0, 0);

}

