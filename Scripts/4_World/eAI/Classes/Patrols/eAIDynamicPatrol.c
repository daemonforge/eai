class eAIDynamicPatrol : eAIPatrol
{
	vector m_Position;
	autoptr array<vector> m_Waypoints;
	eAIWaypointBehavior m_WaypointBehaviour;
	float m_MinimumRadius;
	float m_MaximumRadius;
	float m_DespawnRadius; // m_MaximumRadius + 10%
	int m_NumberOfAI;
	string m_Loadout;

	eAIGroup m_Group;
	int m_LastSpawnIndex;

	/**
	 * @brief Creates a dynamic patrol which spawns a patrol under the right conditions.
	 * 
	 * @param pos the position that the trigger distance is calculated from
	 * @param waypoints an array of points which the patrol will traverse
	 * @param behaviour how the waypoints will be traversed
	 * @param loadout the loadout each member is given @todo change to AI "type" which may have a different FSM/Goal tree
	 * @param count the number of ai to be spawned in the patrol
	 * @param minR miminum distance between the patrol and nearest player for a patrol to not (re)spawn
	 * @param maxR maximum distance between the patrol and nearest player for a patrol to (re)spawn
	 * 
	 * @return the patrol instance
	 */
	static eAIDynamicPatrol Create(vector pos, array<vector> waypoints, eAIWaypointBehavior behaviour, string loadout = "SoldierLoadout.json", int count = 1, bool autoStart = true, float minR = 300, float maxR = 800)
	{
		eAIDynamicPatrol patrol;
		Class.CastTo(patrol, ((typename)eAIDynamicPatrol).Spawn());
		patrol.m_Position = pos;
		patrol.m_Waypoints = waypoints;
		patrol.m_WaypointBehaviour = behaviour;
		patrol.m_MinimumRadius = minR;
		patrol.m_MaximumRadius = maxR;
		patrol.m_DespawnRadius = maxR * 1.1;
		patrol.m_NumberOfAI = count;
		patrol.m_Loadout = loadout;
		if (autoStart) patrol.Start();
		return patrol;
	}

	private eAIBase SpawnAI(vector pos)
	{
		eAIBase ai;
		if (!Class.CastTo(ai, GetGame().CreateObject(GetRandomAI(), pos))) return null;

		ai.SetPosition(pos);

		HumanLoadout.Apply(ai, m_Loadout);
				
		return ai;
	}

	private bool IsGroupDestroyed()
	{
		for (int i = 0; i < m_Group.Count(); i++)
		{
			if (m_Group.GetMember(i) && m_Group.GetMember(i).IsAlive())
			{
				return false;
			}
		}

		return true;
	}

	override void OnUpdate()
	{
		super.OnUpdate();
		
		if (!m_Group) m_LastSpawnIndex++;

		vector patrolPos = m_Position;
		if (m_Group && m_Group.GetLeader()) patrolPos = m_Group.GetLeader().GetPosition();
		
		autoptr array<Man> players = {};
		GetGame().GetPlayers(players);
		float minimumDistance = 50000.0;
		foreach (auto player : players)
		{
			float dist = vector.Distance(patrolPos, player.GetPosition());
			if (dist < minimumDistance) minimumDistance = dist;
		}

		if (m_Group)
		{
			if (IsGroupDestroyed() || minimumDistance > m_DespawnRadius)
			{
				m_Group.RemoveAllMembers();
				m_LastSpawnIndex = 0;
			}
		}
		else
		{
			if (minimumDistance < m_MaximumRadius && minimumDistance > m_MinimumRadius)
			{
				eAIBase ai = SpawnAI(m_Position);
				m_Group = ai.GetGroup();
				m_Group.SetWaypointBehaviour(m_WaypointBehaviour);
				foreach (vector v : m_Waypoints) m_Group.AddWaypoint(v);

				int count = m_NumberOfAI;
				while (count > 1)
				{
					ai = SpawnAI(m_Position);
					ai.SetGroup(m_Group);
					count--;
				}
			}
		}
	}
};