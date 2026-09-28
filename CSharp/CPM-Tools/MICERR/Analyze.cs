using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MicErr
{
	internal class Analyze
	{
		/// <summary>
		///  MI-C output
		/// </summary>
		/// <param name="filename"></param>
		/// <returns>error coun (>=0: error cnt, -1: file not found, -2: parsing error</returns>
		public int Cerr(string filename)
		{
			string[] lines;
			try
			{
				lines = File.ReadAllLines(filename);
			}
			catch(Exception)
			{
				return -1;
			}

			int errCnt = -2;
			foreach (string line in lines)
			{
				if (string.IsNullOrWhiteSpace(line)) continue;
				Console.WriteLine(line);

				if (line[0] == ';')
				{
					int pos = line.IndexOf(" ");
					if (pos > 1)
					{
						string s = line.Substring(1, pos);
						if (int.TryParse(s, out int err))
						{
							errCnt = err;
						}
					}
				}
			}
			return errCnt;
		}

		/// <summary>
		/// M80 file
		/// </summary>
		/// <param name="filename"></param>
		/// <returns></returns>
		public int Merr(string filename)
		{

			string[] lines;
			try
			{
				lines = File.ReadAllLines(filename);
			}
			catch (Exception)
			{
				return -1;
			}

			int errCnt = -2;
			int warnCnt = 0;
			foreach (string line in lines)
			{
				if (string.IsNullOrWhiteSpace(line)) continue;

				Console.WriteLine(line);
				if (line.Contains("Warning(s)"))
				{
					int pos = line.IndexOf(",");
					if (pos > 0)
					{
						string s = line.Substring(pos + 1, line.Length - pos - 1);
						pos = s.IndexOf(" ");
						if (pos > 0)
						{
							s = s.Substring(0, pos);
							if (int.TryParse(s, out int err))
							{
								warnCnt = err;
							}
						}
					}
				}

				if (line.Contains("No Fatal error(s)"))
				{
					errCnt = 0;
					break;
				}

				if (line.Contains("Fatal error(s)"))
				{
					Console.WriteLine(line);
					int pos = line.IndexOf(" ");
					if (pos > 0)
					{
						string s = line.Substring(0, pos);
						if (int.TryParse(s, out int err))
						{
							errCnt = err;
						}
					}
					break;
				}
			}

			if (warnCnt > 0 && errCnt <= 0) return warnCnt;
			return errCnt + warnCnt;
		}

		/// <summary>
		/// M80 output
		/// </summary>
		/// <param name="filename"></param>
		/// <returns></returns>
		public int Lerr(string filename)
		{
			string[] lines;
			try
			{
				lines = File.ReadAllLines(filename);
			}
			catch (Exception)
			{
				return -1;
			}

			int errCnt = 0;
			foreach (string line in lines)
			{
				if (string.IsNullOrWhiteSpace(line)) continue;
				Console.WriteLine(line);

				if (line.Contains("Undefined Global(s)"))
				{
					string s = line.Trim();
					int pos = s.IndexOf(" ");
					if (pos > 0)
					{
						s = s.Substring(0, pos);
						if (int.TryParse(s, out int err))
						{
							errCnt = err;
						}
					}
				}
			}
			return errCnt;
		}
	}
}
