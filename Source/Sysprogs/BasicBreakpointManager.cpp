#include "cmsys/SystemTools.hxx"
#include "BasicBreakpointManager.h"

Sysprogs::BasicBreakpointManager::BreakpointObject *Sysprogs::BasicBreakpointManager::TryGetBreakpointAtLocation(const std::string &file, int oneBasedLine)
{
	auto it = m_BreakpointsByLocation.find(MakeCanonicalLocation(file, oneBasedLine));

	if (it == m_BreakpointsByLocation.end() || it->second.empty())
		return 0;

	return TryLookupBreakpointObject(*it->second.begin());
}

Sysprogs::BasicBreakpointManager::UniqueBreakpointID Sysprogs::BasicBreakpointManager::CreateBreakpoint(const std::string &file, int oneBasedLine)
{
	auto id = m_NextID++;
	auto location = MakeCanonicalLocation(file, oneBasedLine);
	if (location.second.empty())
		return 0;
	m_BreakpointsByLocation[location].insert(id);
	m_BreakpointsByID[id] = std::make_unique<BreakpointObject>(id, location);
	return id;
}

void Sysprogs::BasicBreakpointManager::DeleteBreakpoint(UniqueBreakpointID id)
{
	auto it = m_BreakpointsByID.find(id);
	if (it == m_BreakpointsByID.end())
		return;

	// We may want to clear the remove the location record in case it was the last breakpoint, but it should not cause any noticeable delays.
	m_BreakpointsByLocation[it->second->Location].erase(id);
	m_BreakpointsByID.erase(it);
}

Sysprogs::BasicBreakpointManager::BreakpointObject *Sysprogs::BasicBreakpointManager::TryLookupBreakpointObject(UniqueBreakpointID id)
{
	auto it = m_BreakpointsByID.find(id);
	if (it == m_BreakpointsByID.end())
		return nullptr;
	return it->second.get();
}

Sysprogs::BasicBreakpointManager::CanonicalFileLocation Sysprogs::BasicBreakpointManager::MakeCanonicalLocation(const std::string &file, int oneBasedLine)
{
	auto it = m_CanonicalPathMap.find(file);
	if (it != m_CanonicalPathMap.end())
		return CanonicalFileLocation(oneBasedLine, it->second);

	std::string canonicalPath = cmsys::SystemTools::GetRealPath(file);
	m_CanonicalPathMap[file] = canonicalPath;
	return CanonicalFileLocation(oneBasedLine, canonicalPath);
}
