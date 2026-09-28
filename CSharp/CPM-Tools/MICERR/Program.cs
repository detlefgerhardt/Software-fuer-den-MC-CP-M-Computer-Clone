using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.ConstrainedExecution;
using System.Text;
using System.Threading.Tasks;

namespace MicErr
{
	internal class Program
	{
		static void Main(string[] args)
		{
			Analyze analyze = new Analyze();
			int err = 0;
			//int errCnt = 0;
			foreach (string fn in args)
			{
				string ext = Path.GetExtension(fn);
				Console.WriteLine(Delim(fn));
				switch (ext.ToLower())
				{
					case ".cerr":
						err += Ana(analyze.Cerr, fn);
						break;
					case ".merr":
						err += Ana(analyze.Merr, fn);
						break;
					case ".lerr":
						err += Ana(analyze.Lerr, fn);
						break;
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
				Console.ForegroundColor = col;
			}
			Console.WriteLine(Delim());
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
