using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace KOSW.Products.Approach.BuildTools.RcShallow
{
	class ControlID : IComparable<ControlID>
	{
		string myID;
		int myVal;

		public ControlID(string theID, int theVal)
		{
			myID = theID;
			myVal = theVal;
		}

		public string ID { get { return myID; } }

		public int CompareTo(ControlID theOther)
		{
			int aRes = GroupID().CompareTo( theOther.GroupID() );

			if (aRes == 0)
			{
				if (GroupID() == 1000)
					aRes = SortableID().CompareTo(theOther.SortableID());
				else
				{
					aRes = myVal.CompareTo(theOther.myVal);

					if (aRes == 0)
						aRes = myID.CompareTo(theOther.myID);
				}
			}

			return aRes;
		}

		private string SortableID()
		{
			if ( myID.StartsWith("IDL_") )
				return "IDC_" + myID.Substring(4) + "a";

			else if ( myID.StartsWith("IDC_") )
				return myID + "b";

			else if ( myID.StartsWith("IDM_") )
			{
				int aNext = myID.LastIndexOf("_");

				if (aNext > 3)
					return "IDC_" + myID.Substring(4, aNext - 4 + 1) + "Y" + myID.Substring(aNext + 1);
				else
					return myID;
			}

			else if (myID.StartsWith("IDS_"))
			{
				int aNext = myID.LastIndexOf("_");

				if (aNext > 3)
					return "IDC_" + myID.Substring(4, aNext - 4 + 1) + "Z" + myID.Substring(aNext + 1);
				else
					return myID;
			}

			else
				return myID;
		}

		private int GroupID()
		{
			if (myVal < 1000)
				return 0;
			else if (myVal < 32000)
				return 1000;
			else
				return 32000;
		}

		public string PaddedOut()
		{
			if (myID.Length < 31)
				return myID.PadRight(32, ' ') + myVal.ToString();
			else
				return myID +  " " + myVal.ToString();
		}
	}

	class BaseResource
	{
		public string ID { get; set; }
		public string Text { get; set; }

		public static int GetHash(string theID)
		{
			//taken from a sample
			int b = 378551;
			int a = 63689;

			int myHash = 0;

			foreach (char aCur in theID)
			{
				myHash = myHash * a + (int) Char.ToLowerInvariant(aCur);
				a *= b;
			}

			myHash &= 0x7FFFFFFF;

			return myHash;
		}

		public virtual string ToCppClassString()
		{
			StringBuilder aSB = new StringBuilder();

			CppStructRenderer.RenderStructStart(aSB, ID);
			{
				CppStructRenderer.RenderEnumStart(aSB);
					RenderTraitsAsEnum(aSB);
				CppStructRenderer.RenderEnumEnd(aSB);

				if (ID.StartsWith("IDC"))
				{
					string aTipID = "IDH" + ID.Substring(3);
					CppStructRenderer.RenderTipIdFunc(aSB, aTipID);
				}

				if ( !String.IsNullOrEmpty(Text) )
					CppStructRenderer.RenderTextFunc(aSB, Text);
			}
			CppStructRenderer.RenderStructEnd(aSB);

			return aSB.ToString();
		}

		protected virtual void RenderTraitsAsEnum(StringBuilder theS)
		{
			CppStructRenderer.RenderEnumMember(theS, "ID", ID);
			CppStructRenderer.RenderEnumMember(theS, "Hash", GetHash(ID).ToString());

			if (ID.StartsWith("IDC"))
			{
				string aTipID = "IDH" + ID.Substring(3);
				CppStructRenderer.RenderEnumMember(theS, "TipHash", GetHash(aTipID).ToString());
			}
		}
	}

	class MenuResource : BaseResource
	{
	}

	abstract class WindowResource : BaseResource
	{
		public virtual int Width { get; set; }
		public virtual int Height { get; set; }

		protected override void RenderTraitsAsEnum(StringBuilder theS)
		{
			base.RenderTraitsAsEnum(theS);

			CppStructRenderer.RenderEnumMember(theS, "Width", Width.ToString());
			CppStructRenderer.RenderEnumMember(theS, "Height", Height.ToString());
		}
	}

	class DialogResource : WindowResource
	{
	}

	class ControlResource : WindowResource
	{
		public string ResourceType { get; set; }
		public int X { get; set; }
		public int Y { get; set; }

		public override int Width
		{
			get { if (ResourceType == "ICON") return 0; else return base.Width; }
			set { base.Width = value; }
		}

		public override int Height
		{
			get { if (ResourceType == "ICON") return 0; else return base.Height; }
			set { base.Height = value; }
		}

		protected override void RenderTraitsAsEnum(StringBuilder theS)
		{
			base.RenderTraitsAsEnum(theS);

			CppStructRenderer.RenderEnumMember(theS, "X", X.ToString());
			CppStructRenderer.RenderEnumMember(theS, "Y", Y.ToString());
		}
	}


	static class CppStructRenderer
	{
		public static void RenderStructStart(StringBuilder theS, string theName)
		{
			theS.Append("struct RCENTRY_");
			theS.AppendLine(theName);
			theS.AppendLine("{");
		}

		public static void RenderTextFunc(StringBuilder theS, string theText)
		{
			theS.AppendLine("  static const char * Text()");
			theS.AppendLine("  {");
			{
				theS.Append("    return \"");
				theS.Append(theText);
				theS.AppendLine("\";");
			}
			theS.AppendLine("  }");
		}

		public static void RenderTipIdFunc(StringBuilder theS, string theID)
		{
			theS.AppendLine("  static const char * TipID()");
			theS.AppendLine("  {");
			{
				theS.Append("    return \"");
				theS.Append(theID);
				theS.AppendLine("\";");
			}
			theS.AppendLine("  }");
		}

		public static void RenderEnumStart(StringBuilder theS)
		{
			theS.AppendLine("  enum");
			theS.AppendLine("  {");
		}

		public static void RenderEnumEnd(StringBuilder theS)
		{
			theS.AppendLine("  };");
		}

		public static void RenderStructEnd(StringBuilder theS)
		{
			theS.AppendLine("};");
		}

		public static void RenderEnumMember(StringBuilder theS, string theName, string theValue)
		{
			theS.Append("    ");
			theS.Append(theName);
			theS.Append(" = ");
			theS.Append(theValue);

			theS.AppendLine(",");
		}
	}

	class DialogProcessor
	{
		static readonly Regex ourRX_Dialog = new Regex
		(
			@"(\w+)(\s+DIALOGEX\s+\d+,\s+\d+,)\s+(\d+),\s+(\d+)(.*?END\b)",
			RegexOptions.Compiled | RegexOptions.Singleline
		);

		static readonly Regex ourRX_Control = new Regex
		(
			@"(LTEXT|CTEXT|RTEXT|PUSHBUTTON|DEFPUSHBUTTON|CONTROL|ICON)(\s+)""(.*?)"",(\w+)(.+?)(\d+),(\d+),(\d+),(\d+)",
			RegexOptions.Compiled
		);

		static readonly Regex ourRX_EditControl = new Regex
		(
			@"(EDITTEXT|COMBOBOX|LISTBOX)(\s+)(\w+),(\d+),(\d+),(\d+),(\d+)",
			RegexOptions.Compiled
		);

		static readonly Regex ourRX_Hyphenation = new Regex
		(
			",(\\n|\\r\\n|\\r)\\s+",
			RegexOptions.Compiled | RegexOptions.Singleline
		);

		MatchEvaluator myEv_Control;
		MatchEvaluator myEv_Dialog;
		MatchEvaluator myEv_EditControl;

		List<string> myExclControlIDs;
		List<string> myExclDialogIDs;

		IDictionary<string, BaseResource> myResourceDict;

		public DialogProcessor(string theExcludeIDs, IDictionary<string, BaseResource> theResourceDict)
		{
			myEv_Dialog = new MatchEvaluator(Handler_Replace_Dialog);
			myEv_Control = new MatchEvaluator(Handler_Replace_Control);
			myEv_EditControl = new MatchEvaluator(Handler_Replace_EditControl);

			myExclControlIDs = new List<string>();
			myExclDialogIDs = new List<string>();

			myResourceDict = theResourceDict;

			if (!string.IsNullOrEmpty(theExcludeIDs))
			{
				string[] aExcl = theExcludeIDs.Split(',');

				foreach (string aIt in aExcl)
				{
					if ( aIt.StartsWith("dlg:") )
						myExclDialogIDs.Add(aIt.Substring(4));
					else
						myExclControlIDs.Add(aIt);
				}
			}

			myExclControlIDs.Sort();
			myExclDialogIDs.Sort();
		}

		public string Process(string theString)
		{
			string aMergedLines = ourRX_Hyphenation.Replace(theString, ",");

			return ourRX_Dialog.Replace(aMergedLines, myEv_Dialog);
		}

		private string Handler_Replace_Dialog(Match theM)
		{
			DialogResource aDlg = new DialogResource();
			aDlg.ID = theM.Groups[1].Value;
			aDlg.Text = string.Empty;	//TODO
			aDlg.Width = int.Parse( theM.Groups[3].Value );
			aDlg.Height = int.Parse( theM.Groups[4].Value );

			if ( !myResourceDict.ContainsKey(aDlg.ID) )
				myResourceDict.Add(aDlg.ID, aDlg);

			StringBuilder aSB = new StringBuilder();
			aSB.Append(aDlg.ID);
			aSB.Append(theM.Groups[2]);
			aSB.Append(' ');
			aSB.Append(aDlg.Width);
			aSB.Append(',');
			aSB.Append(' ');
			aSB.Append(aDlg.Height);

			if (myExclDialogIDs.BinarySearch(aDlg.ID) >= 0)
				aSB.Append(theM.Groups[5].Value);
			else
			{
				string aTemp = ourRX_Control.Replace(theM.Groups[5].Value, myEv_Control);
				aTemp = ourRX_EditControl.Replace(aTemp, myEv_EditControl);

				aSB.Append(aTemp);
			}

			return aSB.ToString();
		}

		private string Handler_Replace_Control(Match theM)
		{
			ControlResource aC = new ControlResource();
			aC.ResourceType = theM.Groups[1].Value;
			aC.Text = theM.Groups[3].Value;
			aC.ID = theM.Groups[4].Value;
			aC.X = int.Parse(theM.Groups[6].Value);
			aC.Y = int.Parse(theM.Groups[7].Value);
			aC.Width = int.Parse(theM.Groups[8].Value);
			aC.Height = int.Parse(theM.Groups[9].Value);

			if ( aC.ID.StartsWith("IDC_") || aC.ID.StartsWith("IDL_") )
				if (!myResourceDict.ContainsKey(aC.ID))
					myResourceDict.Add(aC.ID, aC);

			StringBuilder aS = new StringBuilder();

			aS.Append(aC.ResourceType);
			aS.Append( theM.Groups[2].Value );	//whitespace after resource type
			aS.Append( "\"" );

			if (aC.ID.EndsWith("_") || myExclControlIDs.BinarySearch(aC.ID) >= 0)
				aS.Append(aC.Text);


			aS.Append("\",");
			aS.Append(aC.ID);

			aS.Append(theM.Groups[5].Value);

			aS.Append(aC.X.ToString());
			aS.Append(',');

			aS.Append(aC.Y.ToString());
			aS.Append(',');

			aS.Append(aC.Width.ToString());
			aS.Append(',');

			aS.Append(aC.Height.ToString());

			return aS.ToString();
		}
		

		private string Handler_Replace_EditControl(Match theM)
		{
			ControlResource aC = new ControlResource();
			aC.ResourceType = theM.Groups[1].Value;
			aC.ID = theM.Groups[3].Value;
			aC.X = int.Parse(theM.Groups[4].Value);
			aC.Y = int.Parse(theM.Groups[5].Value);
			aC.Width = int.Parse(theM.Groups[6].Value);
			aC.Height = int.Parse(theM.Groups[7].Value);

			if ( aC.ID.StartsWith("IDC_") || aC.ID.StartsWith("IDL_") )
				if (!myResourceDict.ContainsKey(aC.ID))
					myResourceDict.Add(aC.ID, aC);

			StringBuilder aS = new StringBuilder();

			aS.Append(aC.ResourceType);
			aS.Append( theM.Groups[2].Value );	//whitespace after resource type

			aS.Append(aC.ID);
			aS.Append(',');

			aS.Append(aC.X.ToString());
			aS.Append(',');

			aS.Append(aC.Y.ToString());
			aS.Append(',');

			aS.Append(aC.Width.ToString());
			aS.Append(',');

			aS.Append(aC.Height.ToString());

			return aS.ToString();
		}
	}

	class MenuProcessor
	{
		IDictionary<string, BaseResource> myResourceDict;

		static readonly Regex ourRX_Menu = new Regex
		(
			"MENUITEM(\\s+)\"(.*?)\",(\\s+)(\\w+)",
			RegexOptions.Compiled
		);

		public MenuProcessor(IDictionary<string, BaseResource> theResourceDict)
		{
			myResourceDict = theResourceDict;
		}

		public string Process(string theString)
		{
			return ourRX_Menu.Replace(theString, Handler_MenuRegex_Match);
		}

		private string Handler_MenuRegex_Match(Match theMatch)
		{
			MenuResource aR = new MenuResource();
			aR.Text = theMatch.Groups[2].Value;
			aR.ID = theMatch.Groups[4].Value;

			if ( !myResourceDict.ContainsKey(aR.ID) )
				myResourceDict.Add(aR.ID, aR);

			return "MENUITEM" + theMatch.Groups[1].Value + "\"f\"," + theMatch.Groups[3].Value + aR.ID;
		}
	}

	class HeaderProcessor
	{
		static readonly Regex ourRX_Define = new Regex
		(
			@"[#]define\s+(ID\w+)\s+(\d+)",
			RegexOptions.Compiled
		);

		IDictionary<string, BaseResource> myResourceDict;

		public HeaderProcessor( IDictionary<string, BaseResource> theResourceDict)
		{
			myResourceDict = theResourceDict;
		}

		public string Process(string theIn)
		{
			List<ControlID> aCtls = new List<ControlID>();
			List<string> aStatic = new List<string>();

			MatchCollection aMatches = ourRX_Define.Matches(theIn);

			string aBeg = null;
			int aEndStart = 0;

			foreach (Match aM in aMatches)
			{
				if (aBeg == null)
					aBeg = theIn.Substring(0, aM.Index);

				string aID = aM.Groups[1].Value;
				string aVal = aM.Groups[2].Value;

				int aValInt = int.Parse(aVal);

				ControlID aCtl = new ControlID(aID, aValInt);
				aCtls.Add(aCtl);

				aEndStart = aM.Index + aM.Length;
			}

			aCtls.Sort();

			StringBuilder aSb = new StringBuilder(theIn.Length);

			aSb.Append(aBeg);

			foreach (ControlID aIt in aCtls)
			{
				aSb.Append("#define ");
				aSb.AppendLine(aIt.PaddedOut());

				if ( !myResourceDict.ContainsKey(aIt.ID) )
				{
					BaseResource aR = new BaseResource();
					aR.ID = aIt.ID;

					myResourceDict.Add(aR.ID, aR);
				}
			}

			string aEnd = theIn.Substring(aEndStart);
			aSb.Append( aEnd.TrimStart() );

			return aSb.ToString();
		}
	}

	class Program
	{
		static readonly string ourHelpString = 
@"
RcShallow.exe (c) KO Software
    Strips strings from .RC files.

Usage:
    RcShallow -i [Input_File] -o [Output_File] (-x [Skip_ID1,Skip_ID2,...])
    RcShallow -h

Options:
    -i or --input     (required) Data immediately after the option
                      specifies the name of the file to read from.

    -o or --output    (required) Data immediately after the option
                      specifies the name of the file to write
                      processed data to.

    -x or --exclude   (optional) Data immediately after the option
                      specifies a comma-separated list of IDs that
                      will not get processed. No spaces are allowed.

    -h or --help      Display this help.

    -p or --process   (optional) Data immediately after the option
                      specifies the name of the header file to
                      sort control IDs in.";


		[STAThread]
		static void Main(string[] args)
		{
			Environment.ExitCode = Run(args);
		}


		static int Run(string[] args)
		{
			if(args.Length == 1)
			{
				if(args[0] == "-h" || args[0] == "--help")
					Console.Out.WriteLine(ourHelpString);

				return 1;
			}
			else if(args.Length < 4)
			{
				Console.Error.WriteLine("ERROR: Too few arguments. Launch with '-h' or '--help' for usage example.");
				
				return 1;
			}

			string aInput = null, aOutput = null, aExclude = null, aProcess = null, aCompile = null;

			for (int i = 0; i < args.Length; i+=2)
			{
				if (args[i] == "-i" || args[i] == "--input")
					aInput = args[i + 1];

				else if (args[i] == "-o" || args[i] == "--output")
					aOutput = args[i + 1];

				else if (args[i] == "-x" || args[i] == "--exclude")
					aExclude = args[i + 1];

				else if (args[i] == "-p" || args[i] == "--process")
					aProcess = args[i + 1];

				else if (args[i] == "-c" || args[i] == "--compile")
					aCompile = args[i + 1];
			}

			bool aParamsValid = true;

			if(string.IsNullOrEmpty(aInput))
			{
				Console.Error.WriteLine("ERROR: Input file not specified. Launch with '-h' or '--help' for usage example.");
				aParamsValid = false;
			}
			else
			{
				if(aInput.StartsWith("\""))
					aInput = aInput.Substring(1);

				if(aInput.EndsWith("\""))
					aInput = aInput.Substring(0, aInput.Length-1);

				if(!File.Exists(aInput))
				{
					Console.Error.WriteLine("ERROR: Input file does not exist.");
					aParamsValid = false;
				}
			}

			if(string.IsNullOrEmpty(aOutput))
			{
				Console.Error.WriteLine("ERROR: Output file not specified. Launch with '-h' or '--help' for usage example.");
				aParamsValid = false;
			}
			else
			{
				if(aInput.StartsWith("\""))
					aInput = aInput.Substring(1);

				if(aInput.EndsWith("\""))
					aInput = aInput.Substring(0, aInput.Length-1);
			}

			if(!aParamsValid)
				return 1;

			// prepare the list of ID exclusions

			Dictionary<string, BaseResource> aResources = new Dictionary<string, BaseResource>();

			DialogProcessor aLst = new DialogProcessor(aExclude, aResources);
			MenuProcessor aMnu = new MenuProcessor(aResources);


			try
			{
				// perform actual reading and replacement

				string aData = null;

				using(StreamReader aS = File.OpenText(aInput))
					aData = aS.ReadToEnd();

				//MessageBox.Show("Debugger can be attached now");

				aData = aLst.Process(aData);
				aData = aMnu.Process(aData);

				using(StreamWriter aS = File.CreateText(aOutput))
					aS.Write(aData);


				if (aProcess != null)
				{
					if (aProcess.StartsWith("\""))
						aProcess = aProcess.Substring(1);

					if (aProcess.EndsWith("\""))
						aProcess = aProcess.Substring(0, aProcess.Length - 1);

					string aHeader = null;

					using (StreamReader aS = File.OpenText(aProcess) )
						aHeader = aS.ReadToEnd();

					HeaderProcessor aHdrPr = new HeaderProcessor(aResources);
					aHeader = aHdrPr.Process(aHeader);

					using (StreamWriter aS = File.CreateText(aProcess))
						aS.Write(aHeader);
				}

				if (aCompile != null)
				{
					using ( StreamWriter aS = File.CreateText(aCompile) )
					{
						aS.WriteLine("#pragma once");
						aS.WriteLine();
						aS.WriteLine("// This is an auto-generated file. Please do not edit it manually as it may be overwritten.");
						aS.WriteLine();

						foreach (var aIt in aResources.Values)
							aS.WriteLine(aIt.ToCppClassString());

						aS.WriteLine();
						aS.WriteLine();
					}
				}
			}
			catch (Exception e)
			{
				Console.Error.WriteLine("ERROR: " + e.Message);
				Console.Error.WriteLine("-------- STACK TRACE--------------");
				Console.Error.WriteLine(e.StackTrace);

				return 1;
			}

			Console.Out.WriteLine("Success");
			return 0;
		}
	}
}
