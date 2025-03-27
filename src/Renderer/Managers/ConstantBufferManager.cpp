#include "stdafx.h"
#include "ConstantBufferManager.h"

void ConstantBufferManager::Destroy()
{
	m_nameToRHIBuffers.clear();
}
