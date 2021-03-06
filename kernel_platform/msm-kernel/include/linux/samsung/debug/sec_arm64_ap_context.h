#ifndef __SEC_ARM64_AP_CONTEXT_H__
#define __SEC_ARM64_AP_CONTEXT_H__

#include <dt-bindings/samsung/debug/sec_arm64_vh_ipi_stop.h>

enum {
	IDX_CORE_EXTRA_SP_EL0 = 0,
	IDX_CORE_EXTRA_SP_EL1,
	IDX_CORE_EXTRA_ELR_EL1,
	IDX_CORE_EXTRA_SPSR_EL1,
	IDX_CORE_EXTRA_SP_EL2,
	IDX_CORE_EXTRA_ELR_EL2,
	IDX_CORE_EXTRA_SPSR_EL2,
	/* */
	IDX_CORE_EXTRA_REGS_MAX,
};

enum {
	IDX_MMU_TTBR0_EL1 = 0,
	IDX_MMU_TTBR1_EL1,
	IDX_MMU_TCR_EL1,
	IDX_MMU_MAIR_EL1,
	IDX_MMU_ATCR_EL1,
	IDX_MMU_AMAIR_EL1,
	IDX_MMU_HSTR_EL2,
	IDX_MMU_HACR_EL2,
	IDX_MMU_TTBR0_EL2,
	IDX_MMU_VTTBR_EL2,
	IDX_MMU_TCR_EL2,
	IDX_MMU_VTCR_EL2,
	IDX_MMU_MAIR_EL2,
	IDX_MMU_ATCR_EL2,
	IDX_MMU_TTBR0_EL3,
	IDX_MMU_MAIR_EL3,
	IDX_MMU_ATCR_EL3,
	/* */
	IDX_MMU_REG_MAX,
};

struct sec_arm64_ap_context {
	struct pt_regs core_regs;
	uint64_t core_extra_regs[IDX_CORE_EXTRA_REGS_MAX];
	uint64_t mmu_regs[IDX_MMU_REG_MAX];
	bool used;
};

#endif /* __SEC_ARM64_AP_CONTEXT_H__ */
