// This is how code works
// 1. Parse the input arguments and determine if they are correct. Print help if needed
// 2. Read the arguments and read the corresponding lmk files into map(STL) of list(STL) of horizons
// 3. Trim the horizons and remove instances where there is no matching B1 for T1, B1 shallower than T1, TB pair is contained in another TB pair etc
// 4. Sweep on the plane and sort the linked list (STL) of the horizon points based on the depth of tops.
// 5. Travel the linked list and calculate the salt thickness.
// 6. Output the salt thickness file. Only output non zero values.

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <algorithm>
#include <sstream>
#include <map>
#include <iterator>

using namespace std;

struct point
{
    int iline;
    int xline;

    point() : iline(), xline() {}
    point(int iline, int xline, float thickness) : iline(iline), xline(xline) {}
};

struct hrzStruct
{
    int iline;
    int xline;
    float topdepth;
    float botdepth;
    short hrzpair;
    
    hrzStruct() : iline(), xline(), topdepth(), botdepth(), hrzpair() {}

    hrzStruct(int iline, int xline, float topdepth, float botdepth, short hrzpair)
    {
        this->iline = iline;
        this->xline = xline;
        this->topdepth = topdepth;
        this->botdepth = botdepth;
        this->hrzpair = hrzpair;
    }
};

bool operator < (const point& left, const point& right)
{
    if (left.iline != right.iline) return left.iline < right.iline;
    else return left.xline < right.xline;
}

bool operator == (const hrzStruct& left, const hrzStruct& right)
{
    return ((left.iline == right.iline) && (left.xline == right.xline) && (left.hrzpair == right.hrzpair));
}

bool SortPredicate_Ascending (const hrzStruct& lhs, const hrzStruct& rhs)
{
    return (lhs.topdepth < rhs.topdepth);
}

int readFiles(int, char* [], map<point, list<struct hrzStruct> >&);
int trimHorizons(map<point, list<struct hrzStruct> >&);
int sweepPlane(map<point, list<struct hrzStruct> >, string);
int parseArguments(int, char* []);
int progUsage (string);


int main(int argc, char* argv[])
{
    if (parseArguments(argc, argv))
    { cout << endl << "Arguments bad. Exiting..." << endl << endl; return 1;}
    else
    {
        cout << "Arguments good... \nWorking on calculating salt thickness..." << endl;
    }
    
    map<point, list<hrzStruct> > hrzMap;   // Map where key is a point and value is a list of struct that has horizon values

    if (readFiles(argc, argv, hrzMap)) { cout << "Exiting." << endl; return 1; }

    trimHorizons(hrzMap);

// Sweep on the plane, find the salt thickness and write to the file;
    string sltThicknessFilename = argv[argc-1];
    sweepPlane(hrzMap, sltThicknessFilename.substr(7));

    cout << "done" << endl << endl;

    return 0;
}

int readFiles(int argc, char* argv[], map<point, list<hrzStruct> > & hrzMap)
{
    bool lineCntFlag = 0;
    for (int i = 1; i < argc-1; i++)
    {
        ifstream hrzFile(argv[i]);
    
        string line;

        if (hrzFile.is_open())
        {
            long int lineCnt = 0;

            while (getline(hrzFile,line))
            {
                list<hrzStruct> rowList;
                hrzStruct rowStruct;
                vector<float> splitRow;
                point pnt;
                stringstream converterStream(line);

                float num;
                while (converterStream >> num)   splitRow.push_back(num);

                if (splitRow.size() != 5) { cout << "\nBad file \"" << argv[i] << "\"" << endl; return 1;}

                if (splitRow[4] <= 0) continue;
                pnt.iline = splitRow[0];
                pnt.xline = splitRow[1];
                rowStruct.iline = splitRow[0];
                rowStruct.xline = splitRow[1];
                i%2 ? rowStruct.topdepth = splitRow[4] : rowStruct.botdepth = splitRow[4];
                rowStruct.hrzpair = (i+1)/2;

                rowList.push_back(rowStruct);
// Find the iline/xline in the map and then insert there.
                map<point, list<hrzStruct> >::iterator mapFind = hrzMap.find(pnt);
                if (mapFind == hrzMap.end())
                {
                    hrzMap.insert(make_pair(pnt,rowList));
                }
                else
                {
                    list<hrzStruct>::iterator listFind = find((mapFind->second).begin(),(mapFind->second).end(),rowStruct);
                    if (listFind != (mapFind->second).end()) // fill up topdepth & botdepth
                    {
                        if (listFind->topdepth > 0)   listFind->botdepth = splitRow[4];
                        else                          listFind->topdepth = splitRow[4];
                    }
                    else
                    {   mapFind->second.push_back(rowStruct); }
                }
                lineCnt++;
            }
            if (lineCnt > 100000 && lineCntFlag == 0) 
            { 
                cout << "Horizon files are pretty big....it may take a while...\nReading horizon files..." << flush; 
                lineCntFlag = 1;
            }
            hrzFile.close();
        }
        else cout << endl << "Unable to open file..." << argv[i] << endl;
    }
    return 0;
}

int trimHorizons(map<point, list<hrzStruct> > & hrzMap)
{
// Trim the horizons
    cout << "done\nTrimming the horizons..." << flush;
    for (map<point, list<hrzStruct> >::iterator iVRow=hrzMap.begin(); iVRow!=hrzMap.end(); iVRow++)
    {
// Remove where either top or bot is missing
        list<hrzStruct> newhrzStruct(iVRow->second);
        for(list<hrzStruct>::iterator iVColnew=newhrzStruct.begin(); iVColnew!=newhrzStruct.end(); iVColnew++)
        {
            if ((iVColnew->topdepth <= 0) || (iVColnew->botdepth <= 0) || ((iVColnew->botdepth - iVColnew->topdepth) <= 0))
            {
                (iVRow->second).remove(*iVColnew);
            }
        }
// Remove TB pairs where the pair is completely contained in another pair
        newhrzStruct = iVRow->second;
        for(list<hrzStruct>::iterator iVColnew=newhrzStruct.begin(); iVColnew!=newhrzStruct.end(); iVColnew++)
        {
            bool FOUND = 0;
            
            for(list<hrzStruct>::iterator iVCol=(iVRow->second).begin();iVCol != (iVRow->second).end(); iVCol++)
            {
                if ((iVColnew->hrzpair != iVCol->hrzpair) && (iVColnew->topdepth > iVCol->topdepth) && (iVColnew->botdepth < iVCol->botdepth))
                {FOUND = 1; break;}
            }
            if (FOUND == 1) (iVRow->second).remove(*iVColnew);
        }
    }

// Delete the empty map entries
    map<point, list<hrzStruct> > newhrzMap(hrzMap);
    for (map<point, list<hrzStruct> >::iterator iVRownew=newhrzMap.begin(); iVRownew!=newhrzMap.end(); iVRownew++)
    {
        if((iVRownew->second).size() == 0) { hrzMap.erase(iVRownew->first);}
    }

    return 0;
}

int sweepPlane(map<point, list<hrzStruct> > hrzMap, string sltThicknessFilename)
{
// Sort the horizons
    for (map<point, list<hrzStruct> >::iterator iVRow=hrzMap.begin(); iVRow!=hrzMap.end(); iVRow++)
    {
// Sort the list based of depth of tops
        (iVRow->second).sort(SortPredicate_Ascending);
    }
    
    ofstream sltThicknessFile(sltThicknessFilename.c_str());

    if (sltThicknessFile.good())     cout << "done\nWriting salt thickness file \"" << sltThicknessFilename << "\"..." << flush;

    for (map<point, list<hrzStruct> >::iterator iVRow=hrzMap.begin(); iVRow!=hrzMap.end(); iVRow++)
    {
        float dist=0;

        for (list<hrzStruct>::iterator iVCol=(iVRow->second).begin(); iVCol != (iVRow->second).end(); iVCol++)
        {
            list<hrzStruct>::iterator iColNext = ++iVCol; iVCol--;
            
            dist += iVCol->topdepth - iVCol->botdepth;
            if (iColNext != (iVRow->second).end() && iColNext->topdepth < iVCol->botdepth)
            {
                dist += iVCol->botdepth - iColNext->topdepth;
            }
        }
        dist > 0 ? dist = 0 : dist *= -1;

        if (sltThicknessFile.is_open())
        {
            sltThicknessFile << setprecision(2) << fixed << setw(12) << float((iVRow->first).iline) << setprecision(2) << fixed << setw(12) << float((iVRow->first).xline) << setw(12) << "0.00" << setw(12) << "0.00" << setprecision(2) << fixed << setw(12) << dist << endl;
        }
        else
        { cout << "Unable to open output file for writing..." << sltThicknessFile << endl; }
    }
    
    if (sltThicknessFile.is_open()) sltThicknessFile.close();
    return 0;
}

int parseArguments(int argc, char* argv[])
{
    cout << endl;
    if (argc == 1) { cout << "Need horizon names...\n"; progUsage(argv[0]); return 1;}

    if (argc == 2) { cout << "Need at least two horizon names...\n"; progUsage(argv[0]); return 1;}

    string lmk (".lmk"); string hrzName ("");
    for (int i = 1; i < argc-1; i++)
    {
        hrzName = argv[i];

        if (hrzName.size() < 5 || hrzName.compare(hrzName.size()-4,4,lmk) != 0)
        {
            cout << "Please input lmk files...file \""<< hrzName << "\" is not a valid file or doesn't end in lmk"<< endl;
            progUsage(argv[0]);
            return 1;
        }

        ifstream infile(argv[i]);
        if (! infile.good())
        {
            cout << "Please input valid lmk files...file \""<< hrzName << "\" does not exit"<< endl;
            progUsage(argv[0]);
            return 1;
        }
    }

    hrzName = argv[argc-1];
    if (hrzName.size() < 8 || hrzName.compare(0,7,"output=") !=0)
    {
        cout << "Please specify output file...\n"; progUsage(argv[0]); return 1;
    }

    if (argc%2 != 0) {cout << "Need even number of horizon names...\n"; progUsage(argv[0]); return 1;}

    return 0;
}

int progUsage (string progName)
{
    cout << "\nUsage: " << progName << " T1.lmk B1.lmk [T2.lmk] [B2.lmk] .. .. [Tn.lmk] [Bn.lmk] output=outputsaltthicknessfile.lmk\n";

    return 0;
}
