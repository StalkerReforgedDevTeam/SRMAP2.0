// Server-side rank save by player account, so rank survives death, logging off and restarts
class SRZ_RankStore : Managed
{
	protected static ref SRZ_RankStore s_Instance;
	protected ref map<string, int> m_mRanks = new map<string, int>();
	protected const string FILE_PATH = "$profile:SRZ_Ranks.txt";

	//------------------------------------------------------------------------------------------------
	static SRZ_RankStore GetInstance()
	{
		if (!s_Instance)
		{
			s_Instance = new SRZ_RankStore();
			s_Instance.Load();
		}
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	bool GetRank(int playerId, out SR_STALKER_RANK rank)
	{
		string uid = Helpers.GetPlayerGUID(playerId);
		if (uid.IsEmpty())
			return false;

		int value;
		if (!m_mRanks.Find(uid, value))
			return false;

		rank = value;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SetRank(int playerId, SR_STALKER_RANK rank)
	{
		if (!Replication.IsServer())
			return;

		string uid = Helpers.GetPlayerGUID(playerId);
		if (uid.IsEmpty())
			return;

		int current;
		if (m_mRanks.Find(uid, current) && current == rank)
			return;

		m_mRanks.Set(uid, rank);
		Save();
	}

	//------------------------------------------------------------------------------------------------
	protected void Save()
	{
		FileHandle file = FileIO.OpenFile(FILE_PATH, FileMode.WRITE);
		if (!file)
		{
			Print("[SR Ranks] Could not write " + FILE_PATH, LogLevel.ERROR);
			return;
		}

		for (int i = 0; i < m_mRanks.Count(); i++)
		{
			file.WriteLine(string.Format("%1|%2", m_mRanks.GetKey(i), m_mRanks.GetElement(i)));
		}

		file.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void Load()
	{
		if (!Replication.IsServer())
			return;

		FileHandle file = FileIO.OpenFile(FILE_PATH, FileMode.READ);
		if (!file)
			return;

		string line;
		while (file.ReadLine(line) > 0)
		{
			line.Trim();
			int sep = line.IndexOf("|");
			if (sep <= 0)
				continue;

			string uid = line.Substring(0, sep);
			string value = line.Substring(sep + 1, line.Length() - sep - 1);
			m_mRanks.Set(uid, value.ToInt());
		}

		file.Close();
		Print(string.Format("[SR Ranks] Loaded %1 saved ranks", m_mRanks.Count()), LogLevel.NORMAL);
	}
}
