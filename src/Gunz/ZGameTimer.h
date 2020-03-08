#pragma once

class ZGameTimer {
public:
	void Reset() {
		m_nTickTime = GetGlobalTimeMS();
		m_dwUpdateCnt = 0;
	}

	void UpdateTick(u64 nTickTime) {
		m_nTickTime = nTickTime;
		m_dwUpdateCnt++;
	}

	void SetGlobalOffset(i64 nOffset) { m_nGlobalOffset = nOffset; }
	auto GetGlobalTick() const { return m_nTickTime + m_nGlobalOffset; }
	auto GetUpdateCount() const { return m_dwUpdateCnt; }

private:
	u64	m_nTickTime = 0;
	i64	m_nGlobalOffset = 0;
	u32 m_dwUpdateCnt = 0;
};