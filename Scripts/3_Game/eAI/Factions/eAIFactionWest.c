class eAIFactionWest : eAIFaction
{
	void eAIFactionWest()
	{
		m_Name = "West";
	}

	override bool IsFriendly(eAIFaction other)
	{
		if (other.IsInherited(eAIFactionWest)) return true;
		if (other.IsInherited(eAIFactionCivilian)) return true;
		return false;
	}
};