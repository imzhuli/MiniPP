#pragma once
#include "./base.hpp"

struct xUsageAudit : xBinaryMessage {

	X_API_MEMBER virtual void SerializeMembers() override {
		W(AuditId, ValidConnections, DomainErrors, ThirdPartyErrors, UploadSize, DownloadSize);
	}

	X_API_MEMBER virtual void DeserializeMembers() override {
		R(AuditId, ValidConnections, DomainErrors, ThirdPartyErrors, UploadSize, DownloadSize);
	}

	uint32_t AuditId;
	uint32_t ValidConnections;
	uint32_t DomainErrors;
	uint32_t ThirdPartyErrors;
	uint64_t UploadSize;
	uint64_t DownloadSize;
};
