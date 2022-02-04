#pragma once

#include <glad/glad.h>
#include <stdint.h>

namespace flut
{
  class GlQueryRetriever
  {
  public:
    constexpr static uint32_t MAX_SIM_ITERS_PER_FRAME = 20;
    constexpr static uint32_t SIM_STEP_COUNT = 6;

    struct QueryTimings
    {
      float simStempMs[SIM_STEP_COUNT];
      float renderMs = 0.0f;
    };

  private:
    constexpr static uint32_t MAX_FRAME_DELAY = 8;

  public:
    GlQueryRetriever();
    ~GlQueryRetriever();

    void incFrame();
    void incSimIter();

    void beginSimQuery(int stepIdx);
    void beginRenderQuery();
    void endQuery();

    void readFinishedQueries(QueryTimings& timings);

  private:
    uint32_t m_head = 1;
    uint32_t m_tail = 0;
    uint32_t m_currSimIter = 0;
    uint32_t m_simIterCounts[MAX_FRAME_DELAY];
    GLuint m_simQueries[MAX_FRAME_DELAY][MAX_SIM_ITERS_PER_FRAME][SIM_STEP_COUNT];
    GLuint m_renderQueries[MAX_FRAME_DELAY];
  };
}
