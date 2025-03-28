#include <chrono>
#include <thread>

class Timer {
public:
    
    explicit Timer(float targetFramerate = 60.0f)
        : m_targetFrameTime(1.0f / targetFramerate), m_lastTime(std::chrono::high_resolution_clock::now()), m_deltaTime(0.0f)
    {
    }

    void StartFrame()
    {
        m_lastTime = std::chrono::high_resolution_clock::now();
    }

    void FinishFrame()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        m_deltaTime = std::chrono::duration<float>(currentTime - m_lastTime).count();
        m_lastTime = currentTime;
    }

    float GetDeltaTime() 
    {
        return m_deltaTime;
    }

    void WaitForFrame() 
    {
        auto frameEndTime = m_lastTime + std::chrono::duration<float>(m_targetFrameTime);
        auto currentTime = std::chrono::high_resolution_clock::now();

        while (currentTime < frameEndTime) 
        {
            std::this_thread::yield(); //non-blocking wait
            currentTime = std::chrono::high_resolution_clock::now();
        }
    }

    void SetTargetFramerate(float framerate) 
    {
        m_targetFrameTime = 1.0f / framerate;
    }

    float GetTargetFramerate() const 
    {
        return 1.0f / m_targetFrameTime;
    }

private:
    float m_targetFrameTime;
    std::chrono::high_resolution_clock::time_point m_lastTime;
    float m_deltaTime;
};