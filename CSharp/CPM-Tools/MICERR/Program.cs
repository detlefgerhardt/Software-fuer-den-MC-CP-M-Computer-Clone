using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.ConstrainedExecution;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace MicErr
{
	internal class Program
	{
		static void Main(string[] args)
		{
			Analyze analyze = new Analyze();
			int err = 0;
			//int errCnt = 0;

			List<string> lines = new List<string>();

			foreach (string arg in args)
			{
				if (arg.Length > 2 && arg.ToLower().StartsWith("-f"))
				{
					string fname = arg.Substring(2);
					List<string> flist = ReadList(fname);
					if (flist == null) return;
					lines.AddRange(flist);
				}
				else
				{
					lines.Add(arg);
				}
			}

			List<string> errList = new List<string>();
			foreach (string line in lines)
			{
				string ext = Path.GetExtension(line);
				Console.WriteLine(Delim(line));
				int errcnt = 0;
				switch (ext.ToLower())
				{
					case ".cerr":
						errcnt = Ana(analyze.Cerr, line);
						err += errcnt;
						break;
					case ".merr":
						errcnt = Ana(analyze.Merr, line);
						err += errcnt;
						break;
					case ".zerr":
						errcnt = Ana(analyze.Zerr, line);
						err += errcnt;
						break;
					case ".lerr":
						errcnt = Ana(analyze.Lerr, line);
						err += errcnt;
						break;
				}
				if (errcnt > 0)
				{
					errList.Add(line);
				}
			}
			Console.WriteLine(Delim());
			if (err == 0)
			{
				ConsoleColor col = Console.ForegroundColor;
				Console.ForegroundColor = ConsoleColor.Green;
				Console.WriteLine("no errors");
				Console.ForegroundColor = col;
			}
			else
			{
				ConsoleColor col = Console.ForegroundColor;
				Console.ForegroundColor = ConsoleColor.Red;
				Console.WriteLine($"{err} errors and warnings");
				foreach(string line in errList)
				{
					Console.WriteLine(line);
				}
				Console.ForegroundColor = col;
			}
			Console.WriteLine(Delim());
		}

		static List<string> ReadList(string filename)
		{
			string[] lines;
			try
			{
				lines = File.ReadAllLines(filename);
			}
			catch(Exception)
			{
				Console.WriteLine($"Error reading {filename}");
				return null;
			}

			List<string> list = new List<string>();
			foreach (string line in lines)
			{
				string l = line.Trim();
				if (l.Length > 0 && !l.StartsWith("#"))
				{
					list.Add(l);
				}
			}
			return list;
		}

		static int Ana(Func<string, int> func, string fn)
		{
			int err = func(fn);
			ConsoleColor col = Console.ForegroundColor;
			Console.ForegroundColor = ConsoleColor.Red;
			if (err == -1)
			{
				Console.WriteLine($"{fn} not found");
				err = 1;
			}
			if (err == -2)
			{
				Console.WriteLine($"{fn} parse error");
				err = 1;
			}
			Console.ForegroundColor = col;
			return err;
		}

		static string Delim(string msg = null)
		{
			const int LEN = 64;
			if (string.IsNullOrEmpty(msg)) return new string('=', LEN);

			int len = (LEN - msg.Length - 2) / 2 + 1;
			string str = new string('=', len) + " " + msg + " " + new string('=', len);
			return str.Substring(0, LEN);
		}
	}
}
