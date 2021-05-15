modded class DayZPlayerImplement
{
    private autoptr eAITargetInformation m_TargetInformation;

    private eAIGroup m_eAI_Group;
    private int m_eAI_GroupID;
    private int m_eAI_GroupMemberIndex;
    private int m_eAI_GroupMemberIndexSynch;

    void DayZPlayerImplement()
    {
        m_TargetInformation = CreateTargetInformation();

        RegisterNetSyncVariableInt("m_eAI_GroupID");
        RegisterNetSyncVariableInt("m_eAI_GroupMemberIndexSynch");

        m_eAI_GroupID = -1;
        m_eAI_GroupMemberIndex = 0;
        m_eAI_GroupMemberIndex = m_eAI_GroupMemberIndexSynch;
    }

    protected eAITargetInformation CreateTargetInformation()
    {
		//eAITrace trace(this, "CreateTargetInformation");
        return new eAIPlayerTargetInformation(this);
    }

    eAITargetInformation GetTargetInformation()
    {
		//eAITrace trace(this, "GetTargetInformation");
        return m_TargetInformation;
    }

    bool IsAI()
    {
		//eAITrace trace(this, "IsAI");
        return false;
    }

	void SetGroup(eAIGroup group)
    {
		//eAITrace trace(this, "SetGroup", "" + group);

        if (m_eAI_Group == group) return;

        if (m_eAI_Group)
        {
            m_eAI_Group.RemoveMember(this);

            m_eAI_GroupID = -1;
        }

		m_eAI_Group = group;

        if (m_eAI_Group)
        {
            SetGroupMemberIndex(m_eAI_Group.AddMember(this));

            m_eAI_GroupID = m_eAI_Group.GetID();
        }
		
		SetSynchDirty();
	}

	eAIGroup GetGroup()
	{
		//eAITrace trace(this, "GetGroup");
		return m_eAI_Group;
	}

    int GetGroupID()
    {
		//eAITrace trace(this, "GetGroupID");
		return m_eAI_GroupID;
    }

    void SetGroupMemberIndex(int index)
    {
		//eAITrace trace(this, "SetGroupMemberIndex", index.ToString());
		
        m_eAI_GroupMemberIndex = index;
        m_eAI_GroupMemberIndexSynch = index;
		
		SetSynchDirty();
    }

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();

        if (GetGame().IsServer()) return;

        if ((m_eAI_Group && m_eAI_Group.GetID() != m_eAI_GroupID))
        {
            // moved group

            m_eAI_Group.RemoveMember(this);

            m_eAI_Group = eAIGroup.GetGroupByID(m_eAI_GroupID, true);

            m_eAI_Group.Client_SetMemberIndex(this, m_eAI_GroupMemberIndexSynch);
        }
        else if (m_eAI_GroupID != -1 && !m_eAI_Group)
        {
            // joined group

            m_eAI_Group = eAIGroup.GetGroupByID(m_eAI_GroupID, true);

            m_eAI_Group.Client_SetMemberIndex(this, m_eAI_GroupMemberIndexSynch);
        }
        else if (m_eAI_GroupID == -1 && m_eAI_Group)
        {
            // left group

            m_eAI_Group.RemoveMember(this);

            m_eAI_Group = null;
        }
        else if (m_eAI_Group && m_eAI_GroupMemberIndexSynch != m_eAI_GroupMemberIndex)
        {
            // moved within group

            // @note: this has to be the last check as when moving/joining groups
            // the index is out of synch and will be handled in the above checks

            m_eAI_Group.Client_SetMemberIndex(this, m_eAI_GroupMemberIndexSynch);
        }
    }

	override void EEKilled(Object killer)
	{
        m_TargetInformation.OnDeath();

        super.EEKilled(killer);
    }

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
        m_TargetInformation.OnHit();

		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }
};