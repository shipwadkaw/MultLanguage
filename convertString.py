 
import xml.dom.minidom
from pyExcelerator import *
 
# stuff to handle text encodings
import codecs
enc, dec, read_wrap, write_wrap = codecs.lookup('utf-8')
 
import struct
import os
 
RUSSIAN_MAPPING_UTF8_TO_ASCII = [ (1040,'A'),
                  (1042,'B'),
                  (1045,'E'),
                  (1047,'3'),
                  (1050,'K'),
                  (1052,'M'),
                  (1053,'H'),
                  (1054,'O'),
                  (1056,'P'),
                  (1057,'C'),
                  (1058,'T'),
                  (1059,'Y'),
                  (1061,'X') ]
 
EXPORTED_SHEET_NAMES = []
 
 
def IsValidKey(key):
    for char in key:
        if (char == '_'
        or (char >= 'a' and char <= 'z')
        or (char >= 'A' and char <= 'Z')
        or (char >= '0' and char <= '9')):
            continue
        else:
            return False
    return True
 
def parseXLSStrings( inputfile, group_prefix, sheet):
#   try:
        langs = []
        sections = {}
 
        name,vals = parse_xls(inputfile, group_prefix)[sheet]
 
        #count languages
        col = 2
        while vals.has_key( (0,col) ) :
            langs.append( vals[(0, col)] )
            col += 1
        numLangs = col - 2
        assert len(set(langs)) == len(langs), 'found duplicate lang :' + str(langs)
 
        #count strings
        row = 1
        while vals.has_key( (row,0) ) :
            row += 1
        numStrings = row
 
        for line in range(1,numStrings) :
            sectionName = vals[(line, 0)]
            sectionName = group_prefix + sectionName
            if not sections.has_key( sectionName ) :
                sections[sectionName] = { 'name' : sectionName, 'ids' : [] }
                for lang in langs :
                    sections[sectionName][lang] = []
            section = sections[sectionName]
            strId = vals[(line, 1)]
            if(strId in section['ids']):
                print("ERROR: duplicate textID :" + strId + "\t\t In Sheet: " + sectionName )
            if(not IsValidKey(strId)):
                print("ERROR: invalid textID :" + strId + "\t\t In Sheet: " + sectionName )
            section['ids'].append(strId)
            for i in range(numLangs) :
                strn = ' '
                if vals.has_key( (line, i + 2) ) :
                    strn = vals[(line, i + 2)]
                    if isinstance(strn, float):##HACK: For value "5" excel return float.. so we don't want "5.0", is why we convert it in INT then STR
                        strn = int(strn)
                    strn = unicode(strn)
                    # replace non-ascii characters
                    strn = strn.replace( u'\u2026', u'...')
 
                section[langs[i]].append( strn )
 

 
        return ( list(langs), sections.values() )
 
 
 
def ExportBinary( stringsData ):
    global EXPORTED_SHEET_NAMES
    for sheetname, data in stringsData.iteritems():
        (langs, strings) = data
        filename = sheetname.lower() + "."
 
        EXPORTED_SHEET_NAMES.append( sheetname.lower() )
 
        for section in strings:
            for lang in langs:
                completeFilename = filename + lang.lower()
                f = open(completeFilename,'wb')
                numString = len( section[lang] )
                #write total number of strings
                f.write( struct.pack('I', numString) )
 
                numString = len(section[lang])
                for i in range(0,numString):
                    item = section[lang][i]
                    item = item.upper()
                    item = item.replace("\\N", "\n")
                    item = item.replace("\\N", '\n')
                    item = item.replace( u'\u2026' ,'...')
                    item = item.encode('ISO-8859-1')
                    section[lang][i] = item
                #write string data.
                for item in section[lang]:
                    numByte = len(item)
                    f.write( struct.pack('H', numByte) )
                    for i in range(0, numByte):
                        f.write( item[i] )
 
                f.close()
 
def stringsLoadData( inputfile, group_prefix ):
    sheetcounter = 0
    xlWorkBook = parse_xls(inputfile, group_prefix)
    stringsData = {}
    for sheet_name, values in xlWorkBook:
        (langs,strings) = parseXLSStrings(inputfile, group_prefix, sheetcounter)
        sheetcounter += 1
        stringsData[sheet_name] = (langs, strings)
    return stringsData
 
def getBinaryFilename(outFolder,sheet,lang):
    #TODO replace 'bad' filecharacters
    filename = sheet.lower() + "_" + lang.lower() + ".bin"
    return os.path.join( outFolder, filename)
def getIdxMapFilename(outFolder, sheet):
    filename = sheet.lower() + ".id"
    return os.path.join( outFolder, filename)
 
def getAllIdxFilenames( stringsData, outFolder=""):
    targets = []
    for sheetname, data in stringsData.iteritems():
        targets.append(getIdxMapFilename(outFolder, sheetname))
    return targets
 
def getAllBinaryFilenames( stringsData, outFolder=""):
    targets = []
    for sheetname, data in stringsData.iteritems():
        (langs, strings) = data
        for section in strings:
            for lang in langs:
                targets.append(getBinaryFilename(outFolder,sheetname,lang))
 
    return targets
 
 
def ExportUTF8( stringsData, outFolder=""):
    global EXPORTED_SHEET_NAMES
    sheetFiles = {}
    for sheetname, data in stringsData.iteritems():
        (langs, strings) = data
        EXPORTED_SHEET_NAMES.append( sheetname.lower() )
        if not sheetname in sheetFiles:
            sheetFiles[sheetname] = [] 
        numString = 0
        for section in strings:
            for lang in langs:
                completeFilename = getBinaryFilename(outFolder,sheetname,lang)
                sheetFiles[sheetname].append(os.path.basename(completeFilename))
                f = open(completeFilename,'wb')
                numString = len( section[lang] )
                #write total number of strings
                f.write( struct.pack('H', numString) )
 
                numString = len(section[lang])
                for i in range(0,numString):
                    item = section[lang][i]
                    item = item.replace(u'\u2026', u'...')
 
                    #Replace russian unicode with ascii to have a smaller ext_map arrays
                    #for (key,value) in RUSSIAN_MAPPING_UTF8_TO_ASCII:
                        #item = item.replace(unichr(key), unicode(value))
 
                    item = item.encode('utf-8')
                    section[lang][i] = item
 
                #write string data.
                for item in section[lang]:
                    numByte = len(item)
                    f.write( struct.pack('H', numByte) )
                    for i in range(0, numByte):
                        f.write( item[i] )
                f.close()
 
 
        # export id map
        idxMapFileName = getIdxMapFilename(outFolder, sheetname)
        sheetFiles[sheetname].append(os.path.basename(idxMapFileName))
        f = open(idxMapFileName, 'wb')
        f.write(struct.pack('H', numString))
        print "Totle:" + str(numString) + " text in sheet " + sheetname
        for section in strings:
            # export id map
            for item in section['ids']:
                numByte = len(item)
                f.write(struct.pack('H', numByte))
                for i in range(0, numByte):
                    f.write(item[i])
        f.close()
    return sheetFiles


def ExportGameText(infile, outfile):
    return ExportUTF8(stringsLoadData(infile, ""), outfile)
 
