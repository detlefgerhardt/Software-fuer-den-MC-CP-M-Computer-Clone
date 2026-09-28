using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Linq.Expressions;
using System.Security.Authentication.ExtendedProtection;
using System.Text;
using System.Threading.Tasks;

namespace FileSystem
{
	internal class CpmDisk
	{
		private string _filename;

		private byte[] _image;

		private BdosParameter _bdos;

		private List<DirEntry> _dirEntries;

		private int DirStartAddr
		{
			get
			{
				if (_bdos == null) return 0;
				return _bdos.ofs * _bdos.TrackSize;
			}
		}

		public CpmDisk()
		{
		}

		public void Test()
		{
			BdosParameter bdos = new BdosParameter()
			{
				BytePerSector = 128,
				SectorsPerTrack = 40,
				Tracks = 160,
				BlockSize = 2048,
				exm = 0,
				drm = 256 - 1,
				ofs = 4,
			};

			Load("800k.img", bdos);

			/*
			string name = "WS33.PMA";
			//string name = "PIP.COM";
			byte[] file = ReadFile(name, 0);
			File.WriteAllBytes(name, file);

			name = "WS33NEU.PMA";
			//name = "WS33NEU";
			byte[] file2 = File.ReadAllBytes(name);
			WriteFile("WS33NEU.PMA", 0, file2);

			byte[] file3 = ReadFile(name, 0);
			File.WriteAllBytes("WS33NEU2.PMA", file3);
			*/

			byte[] file4 = ReadFile("PIP.COM", 0);
			File.WriteAllBytes("PIP.COM", file4);
			WriteFile("PIPNEU.COM", file4, 0);
			byte[] file5 = ReadFile("PIPNEU.COM", 0);
			File.WriteAllBytes("PIPNEU.COM", file4);

		}

		public bool Load(string filename, BdosParameter bdosPrms)
		{
			bool error = false;
			try
			{
				_image = File.ReadAllBytes(filename);
			}
			catch(Exception)
			{
				error = true;
			}

			if (_image.Length != bdosPrms.DiskSize) error = true;

			if (error)
			{
				_filename = null;
				_image = null;
				_bdos = null;
				return false;
			}

			_filename = filename;
			_bdos = bdosPrms;

			_dirEntries = ReadDir(-1);

			return true;
		}

		public byte[] ReadFile(string filename, int userNumber)
		{
			List<DirEntry> dirEntries = ReadDir(userNumber);
			DirEntry entry = dirEntries.Find(d => d.Filename == filename);
			if (entry == null) return null; // not found

			List<byte> file = new List<byte>();
			int record = 0;
			foreach(Extent e in entry.Extents)
			{
				//Debug.Write($"{e.ExtentNumber}");
				for (int r = 0; r < e.Records; r++)
				{
					file.AddRange(ReadRecord(record, entry.Extents.ToArray()));
					record++;
				}
			}
			return file.ToArray();
		}

		public bool WriteFile(string filename, byte[] file, int usernumber)
		{
			int n = _bdos.dsm < 255 ? 16 : 8;
			int records = IntCeil(file.Length, 128);
			int blocks = IntCeil(records, _bdos.BlockSize / 128);

			byte[] file2 = new byte[records * 128];
			Buffer.BlockCopy(file, 0, file2, 0, file.Length);
			for (int i=file.Length; i<file2.Length; i++)
			{
				file2[i] = 0x1A;
			}

			byte[] allocTable = CreateAllocTable();

			int extents;
			if (n == 8)
			{
				extents = IntCeil(file.Length, 16384);
			}
			else
			{
				extents = IntCeil(file.Length, 32768);
			}

			// records in last extent
			int recPerBlock = _bdos.BlockSize / 128;
			int lastRecords = records %  (n * recPerBlock);

			int[] blockList = new int[blocks];
			// first find blocks
			int last = 0;
			for (int b=0; b<blocks; b++)
			{
				int bl = GetFreeBlock(allocTable, last + 1);
				if (bl == 0) return false; // no free blocks, disk full
				blockList[b] = bl;
				last = bl;
			}

			// first find free extends
			int[] extentList = new int[extents];
			for (int i = 0; i<extents; i++)
			{
				extentList[i] = 0;
			}
			last = 0;
			for (int e = 0; e < extents; e++)
			{
				int ex = GetFreeExtend(last + 1);
				if (ex == -1) return false; // direcory full
				extentList[e] = ex;
				last = ex;
			}

			// than modify
			for (int b = 0; b < blocks; b++)
			{
				allocTable[blockList[b]] = 1;
			}

			int cnt = 0;

			//int recordNo = 0;
			int extentNum = 0;
			//int extentIdx = 0; // index of extents
			int allocNo = 0;
			//int blockno = 0;
			Extent extent = null;
			for (int b = 0; b < blockList.Length; b++)
			{
				Debug.WriteLine($"b={b} / {blockList.Length}");
				if (allocNo == 0)
				{
					extent = new Extent()
					{
						UserNumber = usernumber,
						Filename = filename,
						ExtentNumber = extentNum,
						Readonly = false,
						Records = extentNum < extents - 1 ? recPerBlock * n : lastRecords,
						//Records = 0,
						Alloc = new int[n],
					};
				}
				extent.Alloc[allocNo] = blockList[b];
				//int recs = b < blockList.Length - 1 ? recPerBlock : lastRecords;
				//extent.Records += recs;
				Debug.WriteLine($"{b * _bdos.BlockSize}");
				int fileAddr = b * _bdos.BlockSize;
				int size = _bdos.BlockSize;
				if (fileAddr + size > file2.Length)
				{
					size = file2.Length - fileAddr;
				}
				Debug.WriteLine($"{file2.Length} {fileAddr} {fileAddr + size}");
				int imgAddr = DirStartAddr + blockList[b] * _bdos.BlockSize;
				Debug.WriteLine($"{imgAddr}");
				Buffer.BlockCopy(file2, fileAddr, _image, imgAddr, size);
				allocNo++;
				if (allocNo >= n)
				{
					// extent full, write extent
					byte[] bytes = extent.GetBytes();
					Debug.WriteLine($"extentNum={extentNum} {DirStartAddr + extentList[extentNum] * 32}");
					Buffer.BlockCopy(bytes, 0, _image, DirStartAddr + extentList[extentNum] * 32, 32);
					extentNum++;
					allocNo = 0;
				}
			}
			// letzten extend schreiben;
			if (allocNo > 0)
			{
				// extent full, write extent
				byte[] bytes = extent.GetBytes();
				Debug.WriteLine($"extentNum={extentNum} {DirStartAddr + extentList[extentNum] * 32}");
				Buffer.BlockCopy(bytes, 0, _image, DirStartAddr + extentList[extentNum] * 32, 32);
				extentNum++;
				allocNo = 0;
			}

			return true;
		}

		private byte[] CreateAllocTable()
		{
			// allocation table wihout directory area
			byte[] allocTable = new byte[_bdos.dsm + 1];
			for (int i = 0; i < allocTable.Length; i++)
			{
				allocTable[i] = 0;
			}

			int dirBlocks = (_bdos.drm + 1) * 32 / _bdos.BlockSize;
			for (int i=0; i<dirBlocks; i++)
			{
				allocTable[i] = 1;
			}

			// read all extends
			int addr = DirStartAddr;
			int n = _bdos.dsm < 255 ? 16 : 8;
			for (int i = 0; i < _bdos.drm; i++)
			{
				Extent extent = ReadExtent(addr, -1);
				if (extent == null) continue;

				for (int a = 0; a < n; a++)
				{
					int bl = extent.Alloc[a];
					if (bl != 0) allocTable[bl] = 1;
				}
				addr += 32;
			}
			return allocTable;
		}

		private List<DirEntry> ReadDir(int userNum)
		{
			List<DirEntry> dir = new List<DirEntry>();
			int addr = DirStartAddr;
			for (int i = 0; i < _bdos.drm + 1; i++)
			{
				Extent extent = ReadExtent(addr, userNum);
				if (extent == null) continue;

				Debug.WriteLine(extent);
				DirEntry entry = dir.Find(d => d.Filename == extent.Filename);
				if (entry == null)
				{
					dir.Add(new DirEntry(extent));
				}
				else
				{
					entry.AddExtent(extent);
				}

				addr += 32;
			}

			return dir;
		}

		private Extent ReadExtent(int addr, int userNum)
		{
			int status = _image[addr];
			if (status == 0xE5) return null;

			if (status > 15) return null; // invalid user number
			if (userNum != -1 && userNum != status) return null; // wrong user number

			Extent extent = new Extent();

			extent.UserNumber = status;

			string name = "";
			for (int i = 0; i < 11; i++)
			{
				name += (char)(_image[addr + i + 1] & 0x7F);
			}
			extent.Filename = name.Substring(0, 8).Trim() + "." + name.Substring(8, 3).Trim();

			extent.Readonly = (_image[addr + 9] & 0x80) != 0;

			int ex = _image[addr + 12];
			int s2 = _image[addr + 14];
			extent.ExtentNumber = ((s2 * 32) + ex) / (_bdos.exm + 1);

			int rc = _image[addr + 15];
			extent.Records = (ex & _bdos.exm) * 128 + rc;

			int[] al;
			if (_bdos.dsm < 255)
			{
				al = new int[16];
				for (int i = 0; i < 16; i++)
				{
					al[i] = _image[addr + 16 + i];
				}
			}
			else
			{
				al = new int[8];
				for (int i = 0; i < 8; i++)
				{
					al[i] = _image[addr + 16 + i * 2] + 256 * _image[addr + 16 + i * 2 + 1];
				}
			}
			extent.Alloc = al;

			return extent;
		}

		private byte[] ReadRecord(int record, Extent[] extents)
		{
			//int f = _bdos.BlockSize / 128;
			int bidx = record / (_bdos.BlockSize / 128);
			int ro = record % (_bdos.BlockSize / 128);
			int n = _bdos.dsm < 255 ? 16 : 8;
			int b = extents[bidx / n].Alloc[bidx % n];
			int addr =
					DirStartAddr +				// track offset
					b * _bdos.BlockSize +		// block
					ro * 128;					// record
			Debug.WriteLine($"{record} {bidx / n} {bidx} {addr:X06}");
			return ReadBytes(addr, 128);
		}

		private byte[] ReadBytes(int addr, int length)
		{
			//Debug.WriteLine($"{addr:X06}");
			byte[] buffer = new byte[length];
			Buffer.BlockCopy(_image, addr, buffer, 0, length);
			return buffer;
		}

		private int GetFreeExtend(int start)
		{
			int addr = DirStartAddr + start * 32;
			for (int i = start; i < _bdos.drm + 1; i++)
			{
				if (_image[addr] == 0xE5) return i;
				addr += 32;
			}
			return -1;
		}

		private int GetFreeBlock(byte[] allocTable, int start)
		{
			for (int i = start; i < allocTable.Length; i++)
			{
				if (allocTable[i] == 0) return i;
			}
			return 0;
		}


		private static int IntCeil(int arg1, int arg2)
		{
			int result = arg1 / arg2;
			if ((arg1 % arg2) != 0) result++;
			return result;
		}
	}

	internal class BdosParameter
	{
		public int BytePerSector { get; set; }

		public int SectorsPerTrack { get; set; }

		public int Tracks { get; set; }

		public int BlockSize { get; set; }

		public int TrackSize => BytePerSector * SectorsPerTrack;

		public int DiskSize => BytePerSector * SectorsPerTrack * Tracks;

		// number of blocks - 1
		public int exm { get; set; }

		// number of directory entries - 1
		public int drm { get; set; }

		// number of boot tracks
		public int ofs { get; set; }

		// 
		public int dsm => DiskSize / BlockSize - 1;

	}

	internal class DirEntry
	{
		public string Filename { get; set; }

		public int UserNumber { get; set; }

		public bool Readonly { get; set; }

		public List<Extent> Extents { get; set; }

		public List<int> Alloc { get; set; }

		public void AddExtent(Extent extent)
		{
			Extents.Add(extent);

			foreach(int i in extent.Alloc)
			{
				if (i > 0 ) Alloc.Add(i);
			}
		}

		public DirEntry(Extent extent)
		{
			Alloc = new List<int>();
			Extents = new List<Extent>();

			Filename = extent.Filename;
			UserNumber = extent.UserNumber;
			Readonly = extent.Readonly;
			AddExtent(extent);
		}

		public override string ToString()
		{
			return $"{Filename} {UserNumber} {Extents.Count} {Alloc.Count} {Readonly}";
		}
	}

	internal class Extent
	{
		public string Filename { get; set; }

		public int UserNumber { get; set; }

		public bool Readonly { get; set; }

		public int ExtentNumber { get; set; }

		public int Records { get; set; }

		public int[] Alloc { get; set; }

		public byte[] GetBytes()
		{
			byte[] b = new byte[32];

			b[0] = (byte)UserNumber;
			string fi = Path.GetFileNameWithoutExtension(Filename).PadRight(8);
			string ex = Path.GetExtension(Filename);
			if (ex.StartsWith(".")) ex = ex.Substring(1);
			ex = ex.PadRight(3);
			for (int i=0; i<8; i++)
			{
				b[1 + i] = (byte)fi[i];
			}
			for (int i = 0; i < 3; i++)
			{
				b[9 + i] = (byte)ex[i];
			}
			if (Readonly) b[9] |= 0x80;
			b[12] = (byte)(ExtentNumber & 0x1F);
			b[13] = 0;
			b[14] = (byte)(ExtentNumber / 32);
			b[15] = (byte)Records;
			if (Alloc.Length == 8)
			{
				for (int i = 0; i < 8; i++)
				{
					b[16 + i * 2] = (byte)(Alloc[i] & 0xFF);
					b[16 + i * 2 + 1] = (byte)(Alloc[i] / 256);
				}
			}
			else
			{
				for (int i = 0; i < 16; i++)
				{
					b[16 + i] = (byte)(Alloc[i]);
				}
			}
			return b;
		}

		public override string ToString()
		{
			return $"{Filename} {UserNumber} {ExtentNumber} {Records} {Readonly}";
		}
	}
}
