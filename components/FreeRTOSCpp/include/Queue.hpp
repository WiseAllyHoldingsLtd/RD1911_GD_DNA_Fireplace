#pragma once

#include <type_traits>

#include "FreeRTOS.hpp"
#include "QueueInterface.hpp"

template<class T>
class Queue : public QueueInterface<T>
{
public:
	Queue(uint16_t size)
	{
		static_assert(std::is_pod<T>::value, "Queue only supports Plain old data(POD) types");
		m_handle = xQueueCreate(static_cast<uint32_t>(size), sizeof(T));
		// Add to Registry?
	}

	virtual ~Queue()
	{
		vQueueDelete(m_handle);
		// Unregister from registry?
	}

	virtual bool push(const T &item, uint32_t timeout=portMAX_DELAY)
	{
		return xQueueSendToBack(m_handle, &item, timeout) == pdPASS;
	}
	virtual bool pop(T &item, uint32_t timeout=portMAX_DELAY)
	{
		return xQueueReceive(m_handle, &item, timeout) == pdPASS;
	}

	virtual uint32_t numWaitingItems()
	{
		return uxQueueMessagesWaiting(m_handle);
	}

	virtual uint32_t numAvailableSpace()
	{
		return uxQueueSpacesAvailable(m_handle);
	}

	virtual void reset()
	{
		xQueueReset(m_handle);
	}

protected:
	QueueHandle_t m_handle;
};
