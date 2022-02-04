#pragma once

#include "Core/Window.h"
#include "Core/Runtime.h"
#include "Core/Audio.h"
#include "Engine/Common/Primitive.h"
#include "Engine/Common/Buffer.h"
#include "Engine/Common/Shader.h"
#include "Engine/D3d/Camera.h"
#include "Engine/Cube.h"
#include "FFTW3/fftw3.h"

class Visualizer : public TransparentWindow3D, public Runtime
{
public:
    Visualizer(InstanceHandle instance, std::wstring windowClassName, std::wstring windowTitle);

    virtual HRESULT Create();
    virtual LRESULT CALLBACK Message(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

    LRESULT Render();
    void Update(double delta);

    LRESULT MouseMove(WPARAM wParam, LPARAM lParam);
    LRESULT MouseHover(WPARAM wParam, LPARAM lParam);
    LRESULT MouseLeave(WPARAM wParam, LPARAM lParam);
    LRESULT RightButtonDown(WPARAM wParam, LPARAM lParam);
    LRESULT Command(WPARAM wParam, LPARAM lParam);

    LRESULT Close();

protected:
    virtual HRESULT Resize();

private:
    bool mouseHovering = false;
    bool mouseTracking = false;

    std::vector<FFTWFComplex> spectrum;
    AudioAnalyzer analyzer;

    /*
    Camera camera;
    Cube cube;
    float theta;
    */

    void RenderBorder();
};
