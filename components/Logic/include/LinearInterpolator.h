#pragma once

#include <cstdint>


template <typename T, typename S>
class LinearInterpolator
{
public:
  LinearInterpolator(const T * const xValues, const T * const yValues, const uint16_t numValuePairs) :
  m_xValues(xValues), m_yValues(yValues), m_numValuePairs(numValuePairs)
  {
  }
  
  bool interpolate(const S xValue, S& yValue) const
  {
    bool success = false;
    uint16_t xLowIndex = 0u;
    bool xLowValid = false;
    uint16_t xHighIndex = 0u;
    bool xHighValid = false;
    
    /* Brute force search until proven too slow */
    for (uint16_t i = 0u; i < m_numValuePairs; i++)
    {
      if (xValue >= m_xValues[i])
      {
        xLowValid = true;
        xLowIndex = i;
      }
    }
    for (xHighIndex = xLowIndex; xHighIndex < m_numValuePairs; xHighIndex++)
    {
      if (xValue <= m_xValues[xHighIndex])
      {
        xHighValid = true;
        break;
      }
    }
    if (xLowValid && xHighValid)
    {
      if (xLowIndex == xHighIndex)
      {
        yValue = m_yValues[xLowIndex];
      }
      else
      {
        S xOffset = xValue - m_xValues[xLowIndex];
        S xDiff = m_xValues[xHighIndex] - m_xValues[xLowIndex];
        S yDiff = m_yValues[xHighIndex] - m_yValues[xLowIndex];
        
        yValue = m_yValues[xLowIndex] + (yDiff * xOffset) / xDiff;
      }
      success = true;
    }
    else if (xLowValid)
    {
      yValue = m_yValues[xLowIndex];
    }
    else
    {
      yValue = m_yValues[xHighIndex];
    }
    return success;
  }
  
private:
  const T * const m_xValues;
  const T * const m_yValues;
  const uint16_t m_numValuePairs;
};
