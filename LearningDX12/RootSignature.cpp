#include "RootSignature.h"
#include "Application.h"

#include "DX12LibPCH.h"


RootSignature::RootSignature()
	: m_RootSignatureDesc{}, m_NumDescriptorsPerTable{ 0 },
	m_SamplerTableBitMask(0), m_DescriptorTableBitMask(0)
{
}

RootSignature::RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion)
	: m_RootSignatureDesc{}, m_NumDescriptorsPerTable{ 0 },
	m_SamplerTableBitMask(0), m_DescriptorTableBitMask(0)
{
	SetRootSignatureDesc(rootSignatureDesc, rootSignatureVersion);
}

RootSignature::~RootSignature()
{
	Destroy();
}

void RootSignature::Destroy()
{
	// Go through and delete each descriptor range first
	for (UINT i = 0; i < m_RootSignatureDesc.NumParameters; ++i)
	{
		const D3D12_ROOT_PARAMETER1& rootParameter = m_RootSignatureDesc.pParameters[i];
		if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			delete[] rootParameter.DescriptorTable.pDescriptorRanges;
		}
	}

	// Delete the root parameters now that the descriptor tables have been cleared
	delete[] m_RootSignatureDesc.pParameters;
	m_RootSignatureDesc.pParameters = nullptr;
	m_RootSignatureDesc.NumParameters = 0;

	// Delete the samplers
	delete[] m_RootSignatureDesc.pStaticSamplers;
	m_RootSignatureDesc.pStaticSamplers = nullptr;
	m_RootSignatureDesc.NumStaticSamplers = 0;

	// Set all our counters to 0
	m_DescriptorTableBitMask = 0;
	m_SamplerTableBitMask = 0;

	memset(m_NumDescriptorsPerTable, 0, sizeof(m_NumDescriptorsPerTable));
}

void RootSignature::SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc,
	D3D_ROOT_SIGNATURE_VERSION rootSignatureVersion)
{
	// Make sure the previously allocated root signature description is cleared first
	Destroy();

	Microsoft::WRL::ComPtr<ID3D12Device2> device = Application::Get().GetDevice();

	UINT numParameters = rootSignatureDesc.NumParameters;
	D3D12_ROOT_PARAMETER1* pParameters = numParameters > 0 ? new D3D12_ROOT_PARAMETER1[numParameters] : nullptr;

	for (UINT i = 0; i < numParameters; ++i)
	{
		const D3D12_ROOT_PARAMETER1& pRootParameter = rootSignatureDesc.pParameters[i];
		pParameters[i] = pRootParameter;

		if (pRootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			UINT numDescriptorRanges = pRootParameter.DescriptorTable.NumDescriptorRanges;
			D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges = numDescriptorRanges > 0 ? new D3D12_DESCRIPTOR_RANGE1[numDescriptorRanges] : nullptr;

			memcpy(pDescriptorRanges, pRootParameter.DescriptorTable.pDescriptorRanges,
				sizeof(D3D12_DESCRIPTOR_RANGE1) * numDescriptorRanges);

			pParameters[i].DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
			pParameters[i].DescriptorTable.pDescriptorRanges = pDescriptorRanges;

			// Set the bit mask depending on the type of descriptor table
			if (numDescriptorRanges > 0)
			{
				switch (pDescriptorRanges[0].RangeType)
				{
				case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
				case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
					m_DescriptorTableBitMask |= (1 << i);
					break;
				case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
					m_SamplerTableBitMask |= (1 << i);
					break;
				}
			}

			// Count the number of descriptors in the descriptor table
			for (UINT j = 0; j < numDescriptorRanges; ++j)
			{
				m_NumDescriptorsPerTable[i] += pDescriptorRanges[j].NumDescriptors;
			}
		}
	}

	m_RootSignatureDesc.NumParameters = numParameters;
	m_RootSignatureDesc.pParameters = pParameters;

	UINT numStaticSamplers = rootSignatureDesc.NumStaticSamplers;
	D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = numStaticSamplers > 0 ? new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers] : nullptr;

	if (pStaticSamplers)
	{
		memcpy(pStaticSamplers, rootSignatureDesc.pStaticSamplers,
			sizeof(D3D12_STATIC_SAMPLER_DESC) * numStaticSamplers);
	}

	m_RootSignatureDesc.NumStaticSamplers = numStaticSamplers;
	m_RootSignatureDesc.pStaticSamplers = pStaticSamplers;

	D3D12_ROOT_SIGNATURE_FLAGS flags = rootSignatureDesc.Flags;
	m_RootSignatureDesc.Flags = flags;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc;
	versionRootSignatureDesc.Init_1_1(numParameters, pParameters, numStaticSamplers, pStaticSamplers, flags);

	// Serialize the root signature
	Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&versionRootSignatureDesc, rootSignatureVersion,
		&rootSignatureBlob, &errorBlob));

	// Create the root signature finally
	ThrowIfFailed(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
}

uint32_t RootSignature::GetDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const
{
	uint32_t descriptorTableBitMask = 0;
	switch (descriptorHeapType)
	{
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		descriptorTableBitMask = m_DescriptorTableBitMask;
		break;
	case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		descriptorTableBitMask = m_SamplerTableBitMask;
		break;
	}

	return descriptorTableBitMask;
}

uint32_t RootSignature::GetNumDescriptors(uint32_t rootIndex) const
{
	assert(rootIndex < 32);
	return m_NumDescriptorsPerTable[rootIndex];
}