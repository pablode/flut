#include "GlQueryRetriever.hpp"

#include <assert.h>

using namespace flut;

GlQueryRetriever::GlQueryRetriever()
{
  glCreateQueries(GL_TIME_ELAPSED, MAX_FRAME_DELAY, m_renderQueries);

  for (uint32_t i = 0; i < MAX_FRAME_DELAY; i++)
  {
    for (uint32_t j = 0; j < MAX_SIM_ITERS_PER_FRAME; j++)
    {
      glCreateQueries(GL_TIME_ELAPSED, SIM_STEP_COUNT, m_simQueries[i][j]);
    }
  }

  for (uint32_t i = 0; i < MAX_FRAME_DELAY; i++)
  {
    m_simIterCounts[i] = 0;
  }
}

GlQueryRetriever::~GlQueryRetriever()
{
  glDeleteQueries(MAX_FRAME_DELAY, m_renderQueries);

  for (uint32_t i = 0; i < MAX_FRAME_DELAY; i++)
  {
    for (uint32_t j = 0; j < MAX_SIM_ITERS_PER_FRAME; j++)
    {
      glDeleteQueries(SIM_STEP_COUNT, m_simQueries[i][j]);
    }
  }
}

void GlQueryRetriever::incFrame()
{
  m_head = (m_head + 1) % MAX_FRAME_DELAY;
  assert(m_head != m_tail);
  m_simIterCounts[m_head] = 0;
  m_currSimIter = 0;
}

void GlQueryRetriever::incSimIter()
{
  m_currSimIter++;
  m_simIterCounts[m_head]++;
}

void GlQueryRetriever::beginSimQuery(int stepIdx)
{
  assert(m_currSimIter < MAX_SIM_ITERS_PER_FRAME);
  assert(stepIdx < SIM_STEP_COUNT);
  glBeginQuery(GL_TIME_ELAPSED, m_simQueries[m_head][m_currSimIter][stepIdx]);
}

void GlQueryRetriever::beginRenderQuery()
{
  glBeginQuery(GL_TIME_ELAPSED, m_renderQueries[m_head]);
}

void GlQueryRetriever::endQuery()
{
  glEndQuery(GL_TIME_ELAPSED);
}

void GlQueryRetriever::readFinishedQueries(QueryTimings& timings)
{
  // If the last query in the frame is available, this means that all previous
  // queries are available.
  GLuint64 state;
  glGetQueryObjectui64v(m_renderQueries[m_tail], GL_QUERY_RESULT_AVAILABLE, &state);
  if (state == GL_FALSE)
  {
    return;
  }

  glGetQueryObjectui64v(m_renderQueries[m_tail], GL_QUERY_RESULT_NO_WAIT, &state);
  timings.renderMs = state / 1000000.0f;

  // Retrieve all sim step queries and calculate frame average.
  for (uint32_t j = 0; j < SIM_STEP_COUNT; j++)
  {
    timings.simStempMs[j] = 0.0f;

    uint32_t simIterCount = m_simIterCounts[m_tail];
    if (simIterCount == 0)
    {
      continue;
    }

    for (uint32_t i = 0; i < simIterCount; i++)
    {
      glGetQueryObjectui64v(m_simQueries[m_tail][i][j], GL_QUERY_RESULT_NO_WAIT, &state);
      float elapsedMs = state / 1000000.0f;
      timings.simStempMs[j] += elapsedMs;
    }

    timings.simStempMs[j] /= simIterCount;
  }

  m_tail = (m_tail + 1) % MAX_FRAME_DELAY;
}
