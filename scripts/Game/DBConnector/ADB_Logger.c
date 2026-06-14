class ADB_Logger
{
    static void Error(string msg)
    {
		string stackOutput;
		Debug.DumpStack(stackOutput);
		Print("[ADB] " + msg + "\n" + stackOutput, LogLevel.ERROR);
    }

    static void Info(string msg)
    {
		Print("[ADB] " + msg, LogLevel.NORMAL);
    }
}