#include <spargel/base/check.h>
#include <spargel/render/ui_renderer_metal.h>

//
#include <stdio.h>

namespace spargel::render {
    base::UniquePtr<UIRenderer> makeMetalUIRenderer(gpu::GPUContext* context) {
        return base::makeUnique<UIRendererMetal>(context);
    }
    void UIRendererMetal::render() {
        spargel_check(layer_);
        auto command_buffer = [queue_ commandBuffer];
        auto pass_desc = [[MTLRenderPassDescriptor alloc] init];
        pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear;
        pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore;
        pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(0, 1, 1, 1);

        auto drawable = [layer_ nextDrawable];
        pass_desc.colorAttachments[0].texture = drawable.texture;
        auto encoder =
            [command_buffer renderCommandEncoderWithDescriptor:pass_desc];
        [encoder endEncoding];
        [command_buffer presentDrawable:drawable];
        [command_buffer commit];
        [pass_desc release];
    }
}  // namespace spargel::render
