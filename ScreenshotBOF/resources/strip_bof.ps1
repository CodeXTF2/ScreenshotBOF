function strip-bof {
<#
.SYNOPSIS
    Removes debug symbols from a beacon object file

    Heavily dependent on code by Matthew Graeber (@mattifestation)
    Original code: https://www.powershellgallery.com/packages/PowerSploit/1.0.0.0/Content/PETools%5CGet-ObjDump.ps1
    Author: Yasser Alhazmi (@yas_o_h)
    License: BSD 3-Clause

.PARAMETER Path

    Specifies a path to one or more object file locations.

.EXAMPLE

    C:\PS>strip-bof -Path main.obj

#>

 [CmdletBinding()] Param (
        [Parameter(Position = 0, Mandatory = $True)]
        [ValidateScript({ Test-Path $_ })]
        [String]
        $Path
    )

  
  $Code = @'
        using System;
        using System.IO;
        using System.Text;

        namespace COFF
        {


         public class SECTION_HEADER
            {
                public string Name;
                public uint PhysicalAddress;
                public uint VirtualSize;
                public uint VirtualAddress;
                public uint SizeOfRawData;
                public uint PointerToRawData;
                public uint PointerToRelocations;
                public uint PointerToLinenumbers;
                public ushort NumberOfRelocations;
                public ushort NumberOfLinenumbers;
                public uint Characteristics;
                public Byte[] RawData;

                public SECTION_HEADER(BinaryReader br)
                {
                    this.Name = Encoding.UTF8.GetString(br.ReadBytes(8)).Split((Char) 0)[0];
                    this.PhysicalAddress = br.ReadUInt32();
                    this.VirtualSize = this.PhysicalAddress;
                    this.VirtualAddress = br.ReadUInt32();
                    this.SizeOfRawData = br.ReadUInt32();
                    this.PointerToRawData = br.ReadUInt32();
                    this.PointerToRelocations = br.ReadUInt32();
                    this.PointerToLinenumbers = br.ReadUInt32();
                    this.NumberOfRelocations = br.ReadUInt16();
                    this.NumberOfLinenumbers = br.ReadUInt16();
                    this.Characteristics = br.ReadUInt32();
                }
            }


 public class HEADER
            {
                public ushort Machine;
                public ushort NumberOfSections;
                public uint TimeDateStamp;
                public uint PointerToSymbolTable;
                public uint NumberOfSymbols;
                public ushort SizeOfOptionalHeader;
                public ushort Characteristics;

                public HEADER(BinaryReader br)
                {
                    this.Machine =  br.ReadUInt16();
                    this.NumberOfSections = br.ReadUInt16();
                    this.TimeDateStamp = br.ReadUInt32();
                    this.PointerToSymbolTable = br.ReadUInt32();
                    this.NumberOfSymbols = br.ReadUInt32();
                    this.SizeOfOptionalHeader = br.ReadUInt16();
                    this.Characteristics = br.ReadUInt16();
                }
            }
}
'@

    Add-Type -TypeDefinition $Code
    Write-Host "enumerating sections..."
    try {
        $FileStream = [IO.File]::OpenRead($Path)
        $BinaryReader = New-Object IO.BinaryReader($FileStream)
        $CoffHeader = New-Object COFF.HEADER($BinaryReader)

        # Parse section headers
        $SectionHeaders = New-Object COFF.SECTION_HEADER[]($CoffHeader.NumberOfSections)

        for ($i = 0; $i -lt $CoffHeader.NumberOfSections; $i++)
            {
              $SectionHeaders[$i] = New-Object COFF.SECTION_HEADER($BinaryReader)
            
               if($SectionHeaders[$i].Name.Contains("debug")){
                     Write-Host "found debug section.. zeroing it..."
                     $FileStream.Close();
                     $FileStream2 = [IO.File]::OpenWrite($Path)
                     $FileStream2.Seek($SectionHeaders[$i].PointerToRawData, 'Begin')  | Out-Null
                     for($x = 0; $x -lt $SectionHeaders[$i].SizeOfRawData; $x++){
                            $FileStream2.WriteByte(0)
                     }
                     Write-Host "closing stream...";
                     $FileStream2.Close();
                     Write-Host "done!";
                     return;
               }
        }
    } catch  {
        Add-Type -AssemblyName PresentationFramework
        [System.Windows.MessageBox]::Show("error stripping debug symbols: " + $_.ToString());
        return;
    }
}