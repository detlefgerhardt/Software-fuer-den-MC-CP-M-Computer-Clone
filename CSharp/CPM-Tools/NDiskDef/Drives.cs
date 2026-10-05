using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Dynamic;
using System.IO;
using System.Linq;
using System.Runtime.Serialization.Formatters.Binary;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace NDiskDef
{
	/*
	 * Parameters for Diskdef:
	 * physical:
	 * - Drive number (0..n-1)
	 * - BytesPerSector
	 * - SectorsPerTrack (max. 256)
	 * - TracksPerSide (max. 256)
	 * - Sides
	 * - Skew/Interleave
	 * - 0=DD/1=SS
	 * - 0=Minidisk/1=Maxidisk (or HD)
	 * BDOS:
	 * - BlockSize
	 * - DirEntries
	 * - ChkDirEntries
	 * - BootTracks
	 * low level format:
	 * - UseSso
	 * - GapLen
	 * - Filler
	 */

	enum PrmEnum
	{
		Drive = 0,
		PhysBytesPerSectors = 1,
		PhysSectorsPerTrack = 2,
		PhysTracksPerSide = 3,
		PhysSides = 4,
		PhysSkew = 5,
		PhysDriveType = 6,

		BdosBlockSize = 7,
		BdosDirEntries = 8,
		BdosChkDirEntries = 9,
		BdosBootTracks = 10,

		FmtUseSso = 11,
		FmtGapLen = 12,
		FmtFiller = 13
	}

	enum DriveTypes
	{
		MiniHD = 0, // Maxi=0, DD=0 (HD)
		MaxiSD = 1, // Maxi=0, SD=1 (8" IBM)
		MiniDD = 2, // Mini=1, DD=0 (800KBM)
		MaxiDD = 3, // Mini=1, SD=1
		None = 255,
	}

	internal class Drives
	{
		public const int BDOS_SEC_LEN = 128;

		public const int FIX_ALV = 64;	// fix alv for all disk drives to allow CHGDRV

		public const string CMD_NDISKDEF = "ndiskdef";
		public const string CMD_DEFINE = "define";

		private const int OUTFILE_CNT = 5;
		private readonly string[] ext = new string[OUTFILE_CNT] { ".DPB", ".ALV", ".SIG", ".DH", ".FH" };
		private string[] outNames;

		private Dictionary<string, PhysicalParams> defineList;

		private PhysicalParams[] physList;

		private string errorStr;

		public Drives()
		{
		}

		public void Test()
		{
			string example = "NDISKDEF 0,1024,5,80,2,1,MaxiDD , 40,2048,256,256,4";
			List<string>[] lines = new List<string>[OUTFILE_CNT];
			for (int l = 0; l < OUTFILE_CNT; l++)
			{
				lines[l] = new List<string>();
			}
			string err = CmdNdiskdef(example, lines);
			File.WriteAllLines("test1.inc", lines[0]);
			File.WriteAllLines("test2.inc", lines[1]);
			File.WriteAllLines("test3.inc", lines[2]);
		}

		public void Convert(string fromFilename)
		{
			string errorStr = Convert2(fromFilename);
			if (!string.IsNullOrEmpty(errorStr))
			{
				Console.WriteLine($"{errorStr}");
			}
		}

		public string Convert2(string fromFilename)
		{
			physList = new PhysicalParams[Constants.MAX_DRIVES];
			for (int i = 0; i < Constants.MAX_DRIVES; i++)
			{
				physList[i] = null;
			}

			defineList = new Dictionary<string, PhysicalParams>();

			string[] lines;
			try
			{
				lines = File.ReadAllLines(fromFilename);
			}
			catch (Exception)
			{
				return $"Error reading {fromFilename}";
			}

			DateTime now = DateTime.Now;
			List<string>[] outLines = new List<string>[OUTFILE_CNT];
			outNames = new string[OUTFILE_CNT];
			for (int i = 0; i < OUTFILE_CNT; i++)
			{
				outNames[i] = (Path.GetFileNameWithoutExtension(fromFilename) + ext[i]).ToUpper();
				outLines[i] = new List<string>();
				if (i < 3)
				{
					// assembler file
					outLines[i].Add($"; ndiskdef {outNames[i]} created {now:dd.MM.yy HH:mm} by {Constants.ProgName} {Constants.ProgVersion}");
				}
				else
				{
					// C file
					outLines[i].Add($"/* ndiskdef {outNames[i]} created {now:dd.MM.yy HH:mm} by {Constants.ProgName} {Constants.ProgVersion} */");
				}
			}

			int lineCnt = 0;
			foreach (string line in lines)
			{
				lineCnt++;
				string lin = line.Trim().ToLower();
				if (lin.StartsWith(CMD_NDISKDEF))
				{
					errorStr = CmdNdiskdef(line, outLines);
					if (!string.IsNullOrEmpty(errorStr)) return errorStr + $", line {lineCnt}";
				}
				else if (lin.StartsWith(CMD_DEFINE))
				{
					errorStr = CmdDefine(line, outLines);
					if (!string.IsNullOrEmpty(errorStr)) return errorStr + $", line {lineCnt}";
				}
			}

			for (int l = 0; l < 3; l++)
			{
				outLines[l].Add($"; end of {outNames[l]}");
			}	

			AddDrvPrmFooter(outNames[3], defineList, outLines[3]);

			AddDskFrmFooter(outNames[4], defineList, outLines[4]);

			for (int l = 0; l < OUTFILE_CNT; l++)
			{
				try
				{
					File.WriteAllLines(outNames[l], outLines[l].ToArray());
					Console.WriteLine($"{outNames[l]} created.");
				}
				catch (Exception)
				{
					return $"Error writing {outNames[l]}";
				}
			}

			return null;
		}

		private string CmdDefine(string line, List<string>[] lines)
		{
			line = line.Substring(CMD_DEFINE.Length).Trim();
			int pos = line.IndexOf("=");
			if (pos == -1) return "Error in define";

			string fmtName = line.Substring(0, pos - 1).Trim();
			line = line.Substring(pos + 1).Trim();
			if (!line.ToLower().StartsWith(CMD_NDISKDEF)) return "Error in define";
			line = line.Substring(CMD_NDISKDEF.Length).Trim();

			PhysicalParams physPrm = ParseNdiskDef(line, true, out string errorStr);
			if (physPrm == null) return errorStr;

			physPrm.Name = fmtName;
			defineList[fmtName] = physPrm;

			AddDrvPrm(line, physPrm, lines[3]); // part 4
			AddDskFrm(line, physPrm, lines[4]); // part 5

			return null; // no error
		}

		private string CmdNdiskdef(string line, List<string>[] lines)
		{
			line = line.Substring(CMD_NDISKDEF.Length);
			line = line.Trim();

			PhysicalParams physPrm = ParseNdiskDef(line, false, out string errorStr);
			if (physPrm == null) return errorStr;
			physList[physPrm.Drive] = physPrm;

			AddDpb(line, physPrm, lines[0]); // part 1
			AddAlv(line, physPrm, lines[1]); // part 2
			AddSig(line, physPrm, lines[2]); // part 3

			return null; // no error
		}

		private PhysicalParams ParseNdiskDef(string line, bool isDefine, out string errorStr)
		{
			errorStr = null;

			PhysicalParams physPrm = new PhysicalParams(line);
			if (!string.IsNullOrEmpty(physPrm.ErrorStr))
			{
				errorStr = physPrm.ErrorStr;
				return null; // error in phys. parameters
			}

			BdosParams bdosPrm;
			int drive = physPrm.Drive;
			int useDrive = physPrm.UsePrmsFromDrive;
			string fmtName = physPrm.Name;
			if (!string.IsNullOrEmpty(physPrm.UseDefine))
			{
				if (isDefine)
				{
					errorStr = $"Define used in define statement ('{physPrm.UseDefine}')";
					return null;
				}

				if (defineList.ContainsKey(physPrm.UseDefine))
				{
					physPrm = DeepCopy(defineList[physPrm.UseDefine]);
					physPrm.Drive = drive;
					physPrm.Name = fmtName;
					return physPrm;
				}
				else
				{
					errorStr = $"Error: Define '{physPrm.UseDefine}' not found";
					return null;	// define not found
				}
			}
			if (useDrive != -1)
			{
				if (isDefine)
				{
					errorStr = $"Error: Invalid reference in define statement";
					return null;
				}

				// params from other drive
				if (physList[useDrive] == null)
				{
					errorStr = $"Error: Invalid reference ({useDrive})";
					return null;
				}
				physPrm = DeepCopy(physList[useDrive]);
				physPrm.Drive = drive;
				physPrm.UsePrmsFromDrive = useDrive;
				bdosPrm = physPrm.Bdos;
			}
			else
			{
				bdosPrm = new BdosParams(line);
				if (!string.IsNullOrEmpty(bdosPrm.ErrorStr))
				{
					errorStr = bdosPrm.ErrorStr;
					return null;
				}
			}

			if (physPrm.BytesPerSector < 128)
			{
				errorStr = $"Error: bytes/sector < 128 ({useDrive})";
				return null;
			}

			bdosPrm.CalcFromPhysPrm(physPrm);
			if (bdosPrm.Dpb.DsmError)
			{
				errorStr = "Error: invalid calculated DSM value (blocksize to small?)";
				return null;
			}
			else if (bdosPrm.Dpb.DrmError)
			{
				errorStr = "Error: invalid DRM value (more than 16 direcory blocks)";
				return null;
			}

			DiskPrmBlock dpb = bdosPrm.Dpb;

			physPrm.Bdos = bdosPrm;
			return physPrm;
		}

		private void AddDpb(string line, PhysicalParams physPrm, List<string> lines)
		{
			BdosParams bdosPrm = physPrm.Bdos;
			DiskPrmBlock dpb = bdosPrm.Dpb;

			// lines1.Add("");
			lines.Add($"; drive {physPrm.Drive} " +
				$"{physPrm.Name} " +
				$"({physPrm.DiskSize / 1024}K, " +
				$"{physPrm.BytesPerSector} bytes/sec, " +
				$"{physPrm.SectorsPerTrack} sec/trk, " +
				$"{physPrm.TracksPerSide} trks/side, " +
				$"{physPrm.Sides} sides, " +
				$"{physPrm.GetDriveTypeStr()})");
			lines.Add($"; NDISKDEF {physPrm}");
			lines.Add($"; {bdosPrm.GetDiskDef(physPrm.Skew).ToString().ToUpper()}");
			lines.Add($"; generated by {Constants.ProgName} {Constants.ProgVersion}");

			if (physPrm.UsePrmsFromDrive != -1)
			{
				int useDrive = physPrm.UsePrmsFromDrive;
				lines.Add($"DPB{physPrm.Drive}\tEQU DPB{useDrive}\t; same as drive {useDrive}");
				lines.Add($"XLT{physPrm.Drive}\tEQU XLT{useDrive}");
				lines.Add("");
				return;
			}

			lines.Add($"DPB{physPrm.Drive}\tEQU $\t; disk param block");
			lines.Add($"\tDW {dpb.spt}\t; spt");
			lines.Add($"\tDB {dpb.bsh}\t; bsh");
			lines.Add($"\tDB {dpb.blm}\t; blm");
			lines.Add($"\tDB {dpb.exm}\t; exm");
			lines.Add($"\tDW {dpb.dsm}\t; dsm (0..n-1)");
			lines.Add($"\tDW {dpb.drm}\t; drm (0..n-1)");
			lines.Add($"\tDB {dpb.al0}\t; al0");
			lines.Add($"\tDB {dpb.al1}\t; al1");
			lines.Add($"\tDW {dpb.cks}\t; cks");
			lines.Add($"\tDW {dpb.ofs}\t; ofs");
			lines.Add($"\t; paramters for blocking/deblocking");
			lines.Add($"\tDB {bdosPrm.Dpb.psh}\t; psh (phys. shift)");
			lines.Add($"\tDB {bdosPrm.Dpb.phm}\t; phm (phys. mask)");
			lines.Add($"\t; FLO register");
			if (physPrm.DriveType != 255)
			{
				lines.Add($"\tDB {physPrm.FloReg:X02}h\t; Bit5:0=Maxi/1=Mini, Bit4:0=DD/1=SS");
			}
			else
			{
				lines.Add($"\tDB 00h\t; (not used)");
			}
			lines.Add($"\t; physical parameters");
			lines.Add($"\tDW {physPrm.BytesPerSector}\t; bytes/sector");
			lines.Add($"\tDB {physPrm.SectorsPerTrack} - 1\t; sectors/track - 1 (0..n-1)");
			lines.Add($"\tDB {physPrm.TracksPerSide} - 1\t; tracks/side - 1 (0..n-1)");
			lines.Add($"\tDB {physPrm.Sides}\t; sides");
			lines.Add("");

			// skew / interleave
			if (physPrm.Skew > 1)
			{
				int[] skewTable = CalcSkewTable(physPrm.SectorsPerTrack, bdosPrm.Dpb.spt, physPrm.Skew);
				lines.Add($"\t; translation table");
				lines.Add($"XLT{physPrm.Drive}\tEQU\t$");
				foreach (int val in skewTable)
				{
					lines.Add($"\tDB\t{val}");
				}
			}
			else
			{
				lines.Add($"XLT{physPrm.Drive}\tEQU 0\t; no translation table");
			}
			lines.Add("");
		}

		private void AddAlv(string line, PhysicalParams physPrm, List<string> lines)
		{
			DiskPrmBlock dpb = physPrm.Bdos.Dpb;

			lines.Add($"; drive{physPrm.Drive}");
			lines.Add($"ALV{physPrm.Drive}:\tDS {FIX_ALV}\t; needed: {dpb.alv}");
			lines.Add($"CSV{physPrm.Drive}:\tDS {dpb.csv}");
			lines.Add("");
		}

		private void AddSig(string line, PhysicalParams physPrm, List<string> lines)
		{
			DiskPrmBlock dpb = physPrm.Bdos.Dpb;

			char drv = (char)(physPrm.Drive + 'A');

			if (physPrm.Drive == 0)
			{
				lines.Add($"DB '{drv}={physPrm.Name}'");
			}
			else
			{
				lines.Add($"DB ',{drv}={physPrm.Name}'");
			}
		}

		/* include file for CHGDRV.C / PHYDRV.C
		 * 
		 */
		private void AddDrvPrm(string line, PhysicalParams physPrm, List<string> lines)
		{
			DiskPrmBlock dpb = physPrm.Bdos.Dpb;

			char drv = (char)(physPrm.Drive + 'A');

			lines.Add($"/* format {physPrm.Name} {physPrm.DiskSize/1024}KB " +
				$" ({physPrm.BytesPerSector}/{physPrm.SectorsPerTrack}/{physPrm.TracksPerSide}/{physPrm.Sides}) */");
			lines.Add($"drvprm {physPrm.Name} =");
			lines.Add("{");
			lines.Add($"\t\"{physPrm.Name}\",");
			lines.Add($"\t{physPrm.DiskSize / 1024},");
			lines.Add("\t{");
			lines.Add($"\t\t{dpb.spt},\t/* spt */");
			lines.Add($"\t\t{dpb.bsh},\t/* bsh */");
			lines.Add($"\t\t{dpb.blm},\t/* blm */");
			lines.Add($"\t\t{dpb.exm},\t/* exm */");
			lines.Add($"\t\t{dpb.dsm},\t/* dsm */");
			lines.Add($"\t\t{dpb.drm},\t/* drm */");
			lines.Add($"\t\t{dpb.al0},\t/* al0 */");
			lines.Add($"\t\t{dpb.al1},\t/* al1 */");
			lines.Add($"\t\t{dpb.cks},\t/* cks */");
			lines.Add($"\t\t{dpb.ofs},\t/* ofs */");
			lines.Add($"\t\t/* blocking/deblocking and FLO reg */");
			lines.Add($"\t\t{dpb.psh},\t/* psh */");
			lines.Add($"\t\t{dpb.phm},\t/* phm */");
			if (physPrm.DriveType != 255)
			{
				lines.Add($"\t\t0x{physPrm.FloReg:X02},\t/* FLO reg */");
			}
			else
			{
				lines.Add($"\t\t0x00,\t/* FLO reg (not used) */");
			}
			lines.Add($"\t\t/* physical params */");
			lines.Add($"\t\t{physPrm.BytesPerSector},\t/* phylen */");
			lines.Add($"\t\t{physPrm.SectorsPerTrack - 1},\t/* physec */");
			lines.Add($"\t\t{physPrm.TracksPerSide - 1},\t/* phytrk */");
			lines.Add($"\t\t{physPrm.Sides}\t/* sides */");
			lines.Add("\t}");
			lines.Add("};");
			lines.Add("");
		}

		private void AddDrvPrmFooter(string name, Dictionary<string, PhysicalParams> defineList, List<string> lines)
		{
			string s = "";
			int cnt = 0;
			foreach (var item in defineList)
			{
				if (s != "") s += ",";
				s += item.Value.Name;
				cnt++;
			}
			lines.Add($"#define PRMCNT {cnt}");
			lines.Add($"drvprm *prmlist[] = {{{s}}};");
			lines.Add("");
			lines.Add($"/* end of {name} */ ");

		}


		/* include file for NFORM.C
		 * 
		 */

		private void AddDskFrm(string line, PhysicalParams physPrm, List<string> lines)
		{
			if (physPrm.GapLen == 0) return; // no valid low level format

			lines.Add($"/* format {physPrm.Name} {physPrm.DiskSize / 1024}KB " +
				$" ({physPrm.BytesPerSector}/{physPrm.SectorsPerTrack}/{physPrm.TracksPerSide}/{physPrm.Sides}) */");
			lines.Add($"drvfmt {physPrm.Name} =");
			lines.Add("{");
			lines.Add($"\t\"{physPrm.Name}\",");
			lines.Add($"\t{physPrm.DiskSize / 1024},");
			lines.Add("\t{");
			lines.Add($"\t\t{physPrm.BytesPerSector},\t/* bytes/sector */");
			lines.Add($"\t\t{physPrm.SectorsPerTrack},\t/* sectors/track */");
			lines.Add($"\t\t{physPrm.TracksPerSide},\t/* tracks/side */");
			lines.Add($"\t\t{physPrm.Sides - 1},\t/* 0=SS/1=DS */");
			int dd = ((physPrm.DriveType & 0x01)) ^ 0x01; // bit 0, invertiert zu FLO-Reg
			lines.Add($"\t\t{dd},\t/* 0=SD, 1=DD (inverted to FLO reg) */");
			int maxi = ((physPrm.DriveType & 0x02) >> 1) ^ 0x01; // bit 1, invertiert zu FLO-Reg
			lines.Add($"\t\t{maxi},\t/* 0=Mini, 1=Maxi/HD (inverted to FLO reg) */");
			int useSso = physPrm.UseSso ? 1 : 0;
			lines.Add($"\t\t{useSso},\t/* UseSSO */");
			lines.Add($"\t\t{physPrm.GapLen},\t/* gap3 length */");
			lines.Add($"\t\t0x{physPrm.Filler:X02}\t/* filler */");
			lines.Add("\t}");
			lines.Add("};");
			lines.Add("");
		}

		private void AddDskFrmFooter(string name, Dictionary<string, PhysicalParams> defineList, List<string> lines)
		{
			string s = "";
			int cnt = 0;
			foreach (var item in defineList)
			{
				if (item.Value.GapLen > 0)
				{
					if (s != "") s += ",";
					s += item.Value.Name;
					cnt++;
				}
			}
			lines.Add($"#define FMTCNT {cnt}");
			lines.Add($"drvfmt *fmtlist[] = {{{s}}};");
			lines.Add("");
			lines.Add($"/* end of {name} */ ");

		}

		private int[] CalcSkewTable(int physSectorsPerTrack, int bdosSectorsPerTrack, int skew)
		{
			int sectorBase = 1;

			int[] physSkewTable = new int[physSectorsPerTrack];
			//int[] formTable = new int[physSectorsPerTrack];
			bool[] used = new bool[physSectorsPerTrack];

			int pos = 0;

			for (int sector = 0; sector < physSectorsPerTrack; sector++)
			{
				physSkewTable[sector] = pos + sectorBase;
				//formTable[pos] = sector + sectorBase;
				used[pos] = true;

				if (sector == physSectorsPerTrack - 1) break;

				pos = (pos + skew) % physSectorsPerTrack;
				if (used[pos])
				{
					for (int i = 0; i < physSectorsPerTrack; i++)
					{
						if (!used[pos % physSectorsPerTrack])
						{
							break;
						}
						pos++;
					}
				}
			}

			// create logical Skewtable
			int[] logSkewTable = new int[bdosSectorsPerTrack];
			int fact = bdosSectorsPerTrack / physSectorsPerTrack;
			for (int s = 0; s < bdosSectorsPerTrack; s++)
			{
				logSkewTable[s] = physSkewTable[s / fact];
			}
			return logSkewTable;
		}

		private static T DeepCopy<T>(T obj)
		{
			using (MemoryStream stream = new MemoryStream())
			{
				BinaryFormatter formatter = new BinaryFormatter();
				formatter.Serialize(stream, obj);
				stream.Position = 0;

				return (T)formatter.Deserialize(stream);
			}
		}
	}

	[Serializable]
	internal class PhysicalParams
	{
		public int Drive { get; set; }

		public int UsePrmsFromDrive { get; set; }

		public string UseDefine { get; set; }

		public int BytesPerSector { get; set; }

		public int SectorsPerTrack { get; set; }

		public int TracksPerSide { get; set; }

		public int Sides { get; set; }

		public int Skew { get; set; }

		public int DriveType { get; set; }

		public bool UseSso { get; set; }

		public int GapLen { get; set; }

		public int Filler { get; set; }

		public BdosParams Bdos { get; set; }

		private string _name;
		public string Name
		{
			get
			{
				if (!string.IsNullOrEmpty(_name)) return _name;
				return (DiskSize / 1024).ToString() + "K";
			}
			set
			{
				_name = value;
			}
		}

		public int DiskSize => BytesPerSector * SectorsPerTrack * TracksPerSide * Sides;

		public int FloReg => DriveType << 4; // -> bits 4 & 5

		public string ErrorStr { get; set; }

		public PhysicalParams(string prms)
		{
			ErrorStr = ParsePrms(prms);
		}

		private string ParsePrms(string prms)
		{
			string[] parts = prms.Split(',');

			if (parts.Length != 2 && parts.Length != 14)
			{
				return $"Error: Invalid parameter count in NDISKDEF ({parts.Length})";
			}

			int prm;
			string prmStr;
			bool error;
			string errorStr;

			prmStr = parts[(int)PrmEnum.Drive].Trim();
			error = !int.TryParse(prmStr, out prm);
			if (error || prm >= Constants.MAX_DRIVES)
			{
				return $"Error in NDISKDEF paramter {PrmEnum.Drive} ({prmStr})";
			}
			Drive = prm;


			prm = ParsePrm(parts, PrmEnum.PhysBytesPerSectors, out errorStr);
			if (errorStr != null)
			{
				// possible use of define
				UseDefine = parts[(int)PrmEnum.PhysBytesPerSectors].Trim();
				Name = UseDefine;
				return null;
			}

			if (parts.Length == 2)
			{
				// possible reference
				UsePrmsFromDrive = prm;
				return null;
			}

			BytesPerSector = prm;
			UsePrmsFromDrive = -1;

			SectorsPerTrack = ParsePrm(parts, PrmEnum.PhysSectorsPerTrack, out errorStr);
			if (errorStr != null) return errorStr;

			TracksPerSide = ParsePrm(parts, PrmEnum.PhysTracksPerSide, out errorStr);
			if (errorStr != null) return errorStr;

			Sides = ParsePrm(parts, PrmEnum.PhysSides, out errorStr);
			if (errorStr != null) return errorStr;

			Skew = ParsePrm(parts, PrmEnum.PhysSkew, out errorStr);
			if (errorStr != null) return errorStr;

			DriveType = ParseDriveType(parts, PrmEnum.PhysDriveType, out errorStr);
			if (errorStr != null) return errorStr;

			prm = ParsePrm(parts, PrmEnum.FmtUseSso, out errorStr);
			if (errorStr != null) return errorStr;
			if (prm != 0 && prm != 1) return "Invalid UseSSO parameter";
			UseSso = prm == 1;

			GapLen = ParsePrm(parts, PrmEnum.FmtGapLen, out errorStr);
			if (errorStr != null) return errorStr;

			Filler = ParsePrm(parts, PrmEnum.FmtFiller, out errorStr);
			if (errorStr != null) return errorStr;

			return null; // no error
		}

		private int ParsePrm(string[] parts, PrmEnum prmName, out string errorStr)
		{
			string prmStr = parts[(int)prmName].Trim();
			bool error = !int.TryParse(prmStr, out int value);
			if (error)
			{
				errorStr = $"Error in NDISKDEF paramter {prmName} ({prmStr})";
				return 0;
			}
			errorStr = null;
			return value;
		}

		private int ParseDriveType(string[] parts, PrmEnum prmName, out string errorStr)
		{
			errorStr = null;
			string typeStr = parts[(int)PrmEnum.PhysDriveType].Trim();

			foreach (DriveTypes type in Enum.GetValues(typeof(DriveTypes)))
			{
				if (string.Compare(typeStr, type.ToString(), true) == 0) return (int)type;
			}
			errorStr = $"Error in NDISKDEF paramter {prmName} ({typeStr})";
			return 0;
		}

		public string GetDriveTypeStr()
		{
			if (DriveType == 255) return "IDE/CF";

			foreach (DriveTypes type in Enum.GetValues(typeof(DriveTypes)))
			{
				if ((int)type == DriveType) return type.ToString();
			}
			return null;
		}

		public override string ToString()
		{
			return $"{Drive},{BytesPerSector},{SectorsPerTrack},{TracksPerSide},{Sides},{GetDriveTypeStr()} , " +
				$"{Bdos.BlockSize},{Bdos.DirEntries},{Bdos.ChkDirEntries},{Bdos.BootTracks}";
		}
	}

	[Serializable]
	internal class BdosParams
	{
		public int Drive { get; set; }

		public int BlockSize { get; set; }

		public int DirEntries { get; set; }

		public int ChkDirEntries { get; set; }

		public int BootTracks { get; set; }

		public DiskPrmBlock Dpb { get; set; } = new DiskPrmBlock();

		public string ErrorStr { get; set; }

		public BdosParams(string prms)
		{
			ErrorStr = ParsePrms(prms);
		}

		private string ParsePrms(string prms)
		{
			string[] parts = prms.Split(',');

			string errorStr;

			Drive = ParsePrm(parts, PrmEnum.PhysSectorsPerTrack, out errorStr);
			if (errorStr != null) return errorStr;

			BlockSize = ParsePrm(parts, PrmEnum.BdosBlockSize, out errorStr);
			if (errorStr != null) return errorStr;

			DirEntries = ParsePrm(parts, PrmEnum.BdosDirEntries, out errorStr);
			if (errorStr != null) return errorStr;

			ChkDirEntries = ParsePrm(parts, PrmEnum.BdosChkDirEntries, out errorStr);
			if (errorStr != null) return errorStr;

			BootTracks = ParsePrm(parts, PrmEnum.BdosBootTracks, out errorStr);
			if (errorStr != null) return errorStr;

			return null; // no error
		}

		private int ParsePrm(string[] parts, PrmEnum prmName, out string errorStr)
		{
			string prmStr = parts[(int)prmName].Trim();
			bool error = !int.TryParse(prmStr, out int value);
			if (error)
			{
				errorStr = $"Error in NDISKDEF paramter {prmName} ({prmStr})";
				return 0;
			}
			errorStr = null;
			return value;
		}

		public bool CalcFromPhysPrm(PhysicalParams physPrm)
		{
			int BootSize = 2 * physPrm.BytesPerSector * physPrm.SectorsPerTrack;

			Dpb = new DiskPrmBlock();

			// logical sectors per track (=records)
			Dpb.spt = physPrm.BytesPerSector / 128 * physPrm.SectorsPerTrack;

			// block shift factor, 2er Exponent der Blockgroesse
			Dpb.bsh = (int)Math.Round(Math.Log(BlockSize / 128, 2));

			// block length mask, Anzahl der Records per Block - 1
			Dpb.blm = (BlockSize / 128) - 1;

			int tracks = physPrm.TracksPerSide * physPrm.Sides;
			// data storage maximum, Hoechste Blocknummer der Diskette
			Dpb.dsm = ((physPrm.BytesPerSector * (tracks - BootTracks) * physPrm.SectorsPerTrack) / BlockSize) - 1;
			//double trk = ((dsm + 1.0) * Blocksize) / (SecLen * SecTrk) + BootTrk;

			if (Dpb.dsm > 32767 || BlockSize == 1024 && Dpb.dsm > 255)
			{
				Dpb.DsmError = true;
				return false;
			}

			// extent mask, Anzahl der Extents pro Eintrag - 1
			if (Dpb.dsm < 256)
			{
				Dpb.exm = BlockSize / 1024 - 1;
			}
			else
			{
				Dpb.exm = BlockSize / 2048 - 1;
			}

			// directory maximum, Hoechst Eintragsnummer im Directory
			Dpb.drm = DirEntries - 1;

			int dirsize = (Dpb.drm + 1) * 32; // directory size in bytes
			int dav = dirsize / BlockSize; // blocks needed for directory

			//if (Dpb.drm > (BlockSize / 32 * 16) - 1)
			if (dirsize > BlockSize * 16)
			{
				// more than 16 dir blocks
				Dpb.DrmError = true;
				return false;
			}

			// create allocation vector (low/high), one bit for each block
			int alv01 = 0;
			for (int cnt = 0; cnt < dav; cnt++)
			{
				alv01 = alv01 + (1 << (15 - cnt));
			}
			Dpb.al0 = (alv01 & 0xFF00) >> 8; // high byte
			Dpb.al1 = alv01 & 0x00FF; // low byte

			// check vector size, Anzahl der zu pruefenden Directory-Records
			Dpb.cks = ChkDirEntries / 4;

			Dpb.ofs = BootTracks;

			// allocation vector (das ist korrekt so. es ist nicht (Dpb.dsm + 1) / 8)
			Dpb.alv = Dpb.dsm / 8 + 1;
			/*
			if (fixAlv != 0 && Dpb.alv < fixAlv)
			{
				Dpb.alv = fixAlv;
			}
			*/

			Dpb.csv = 0;
			if (Dpb.cks > 0)
			{
				Dpb.csv = (Dpb.drm + 1) / 4;
			}

			// physical record shift factor
			Dpb.psh = (int)Math.Round(Math.Log(physPrm.BytesPerSector / Drives.BDOS_SEC_LEN, 2));

			// physical record mask
			Dpb.phm = physPrm.BytesPerSector / Drives.BDOS_SEC_LEN - 1;

			return true;
		}

		public DiskDefPrm GetDiskDef(int skew)
		{
			return new DiskDefPrm()
			{
				dn = Drive,
				fsc = 0,
				lsc = Dpb.spt - 1,
				skew = skew,
				bls = BlockSize,
				dks = Dpb.dsm + 1,
				dir = Dpb.drm + 1,
				cks = ChkDirEntries,
				ofs = Dpb.ofs
			};
		}
		public override string ToString()
		{
			return $"{Drive} {BlockSize}";
		}
	}

	[Serializable]
	internal class DiskPrmBlock
	{
		public int spt;
		public int bsh;
		public int blm;
		public int exm;
		public int dsm;
		public int drm;
		public int al0;
		public int al1;
		public int cks;
		public int ofs;
		public int alv;
		public int csv;

		public int psh;
		public int phm;

		public bool DsmError = false;
		public bool DrmError = false;
	}

	class DiskDefPrm
	{
		// disk number 0..n-1
		public int dn;
		// first sector number
		public int fsc;
		// last sector number
		public int lsc;
		// skew factor
		public int skew;
		// block size
		public int bls;
		// disk size (in blocks, without boot tracks)
		public int dks;
		// number for dir elements
		public int dir;
		// number for dir to checksum
		public int cks;
		// boot offsett
		public int ofs;

		public override string ToString()
		{
			return $"diskdef {dn},{fsc},{lsc},{skew},{bls},{dks},{dir},{cks},{ofs}";
		}
	}
}