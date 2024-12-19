#pragma once

#include <core/timer_wheel.hpp>
#include <pp_common/base.hpp>

class xComponentAbstract;
class xComponentManagerAbstract;
class xComponentHolder;

class xComponentAbstract : xAbstract {
	friend class xComponentHolder;

protected:
	void SetOuter(void * O) {
		Outer = O;
	}

private:
	void *                      Outer          = this;
	ssize_t                     ReferenceCount = 1;
	xComponentManagerAbstract * Manager        = nullptr;
};
