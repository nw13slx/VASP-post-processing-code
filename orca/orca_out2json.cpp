//this code is used to analyse the Print[ P_MOs ] 1
//output of ORCA. The density of state will be decomposed
//to different element and different angular momentum , s p d f
//this is called PDOS in the solid state dft community
//usage: MO <inputfile name> <outputfile name> 
//author: Lixin Sun nw13mifaso@gmail.com

#include <algorithm>
#include <iostream>
#include <locale>
#include <fstream>          // file I/O suppport
#include <cstdlib>          // support for exit()
#include <stdio.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <time.h>
#include <cmath>
#include <malloc.h>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <sstream>
using namespace std;

#include "math.h"
#include "stdlib.h"       // for random
#include <vector>

//set up the size of array used for storage
#define MAX_ELEMENT 10
#define MAX_M   6
#define MAX_ENERGYLINE 4000
#define TEMP_ENERGYLINE 10

//set up the final plotting region
//the fermi level is shfited to zero
#define EMIN -30
#define EMAX 15

//set up the buffer size for reading
#define MAX_CHARACTER 1000
#define MAX_COLUMN 20

const double Eh2eV=27.2113834;
const double bohr2a=0.529177249;
const double Ehau2eVa=Eh2eV/bohr2a;
const double kcal2eV=4.3363e-2;

bool isxyz(char c){
    if ((c=='x')||(c=='y')||(c=='z')) return true;
    else return false;
}

bool isdigit1(char c){
    if ((c>='0')&&(c<='9')) return true;
    else return false;
}

void gaussian(int grid, double *x, double *y, double x0, double sigma2){
    for (int i=0;i<grid;i++) y[i]=exp(-(x[i]-x0)*(x[i]-x0)/2./sigma2);
}

bool contain_alphabet(char * c){
    for (int i=0;i<strlen(c);i++){
        if (((c[i]>='a')&&(c[i]<='z'))||((c[i]>='A')&&(c[i]<='Z')))
            return true;
    }
    return false;
}

int find_pattern(ifstream &In1,const char *pattern,const char * mode=(const char *)"contains"){
  char temp[MAX_CHARACTER], * pch;
  int beg_pos=In1.tellg();
  bool find=false;
  int beg_line=0;
  if (strcmp(mode,"contains")==0){
    while (In1.good() && !find){
      beg_line=In1.tellg();
      In1.getline(temp,MAX_CHARACTER); 
      pch=strstr(temp,pattern);
      if (pch!=NULL){
        find=true;
      }
    }
  }else{
    while (In1.good() && !find){
      beg_line=In1.tellg();
      In1.getline(temp,MAX_CHARACTER); 
      if (strcmp(temp,pattern)==0) find=true;
    }
  }
  if (!find){
      In1.clear();
      In1.seekg(beg_pos);
      beg_line=NULL;
  }
  return beg_line;
}

int break_line(char *temp,string *content){
  char *pch = strtok (temp," ");
  int column=0;
  while (pch != NULL) {
      content[column]=string(pch);
      column++;
      pch = strtok (NULL, " ");
  }
  pch= NULL;
  return column;
}

int main(int argc, char **argv){

    char temp[MAX_CHARACTER], * pch;
    string* content=new string[MAX_COLUMN];
    int column, line=0;
    bool Print_MO=false;
    
    //locate the beginning of the print_MO
    ifstream In1(argv[1]);
    if ( !In1.good()){
        cout<< " the input file does not exist or is corrupted..."<<endl;
        return 1;
    }
    ofstream json_o(argv[2]);
    if ( !json_o.good()){
        cout<< " the output file does not exist or is corrupted..."<<endl;
        return 1;
    }
    json_o.precision(5);
    json_o<<"{"<<endl;

    json_o<<"\"outputfile\":\""<<argv[1]<<"\","<<endl;
    json_o<<"\"software\":\"ORCA";
    char *pattern="Program Version";
    int v_pos=find_pattern(In1,pattern);
    In1.seekg(v_pos);
    In1.getline(temp,MAX_CHARACTER); 
    break_line(temp,content);
    json_o<<content[2]<<"\","<<endl;

    json_o<<"\"comp_info\":[";
    pattern="Your calculation utilizes ";
    int len_pat=strlen(pattern);
    int u_count=0;
    int u_pos=find_pattern(In1,pattern);
    while (u_pos!=NULL && In1.good()){
      In1.seekg(u_pos);
      In1.getline(temp,MAX_CHARACTER); 
      if (u_count>0){
        json_o<<", ";
      }
      json_o<<"\""<<&temp[len_pat]<<"\"";
      u_count++;
      u_pos=find_pattern(In1,pattern);
    }
    json_o<<"],"<<endl;

    pattern="CARTESIAN COORDINATES (ANGSTROEM)";
    int c_pos=find_pattern(In1,pattern);
    int atomn=0;
    if (c_pos!=NULL){
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      column=break_line(temp,content);
      json_o<<"\"coordinates\":[";
      char species[100000]="\"species\":[";
      while (column==4){
        if (atomn>0){
          json_o<<", ";
          strcat(species,",");
        }
        json_o<<"["<<content[1]<<","<<content[2]<<","<<content[3]<<"]";
        strcat(species,"\"");
        strcat(species,content[0].c_str());
        strcat(species,"\"");
        atomn++;
        In1.getline(temp,MAX_CHARACTER);
        column=break_line(temp,content);
      }
      json_o<<"],"<<endl;
      strcat(species,"],");
      json_o<<species<<endl;
      json_o<<"\"atomn\":"<<atomn<<","<<endl;
    }

    pattern="Hamiltonian:";
    int ham_pos=find_pattern(In1,pattern);
    if (ham_pos!=NULL){
      In1.getline(temp,MAX_CHARACTER);
      column=break_line(temp,content);
      json_o<<"\"hamiltonian\":[";
      int nha=0;
      while(column>0){
        if (temp[1]!=' '){
          if (nha>0) json_o<<",";
          json_o<<"\""<<content[0];
          if (content[column-1].compare("on")!=0){
            json_o<<" "<<content[column-1];
          }
          json_o<<"\"";
          nha++; 
        }
        In1.getline(temp,MAX_CHARACTER);
        column=break_line(temp,content);
      }
      json_o<<"],"<<endl;
    }



    pattern="SCF CONVERGENCE";
    int scf_pos=find_pattern(In1,pattern);
    In1.getline(temp,MAX_CHARACTER);
    In1.getline(temp,MAX_CHARACTER);

    In1.getline(temp,MAX_CHARACTER);
    break_line(temp,content);
    json_o<<"\""<<content[0]<<"_"<<content[1]<<"_"<<content[2]<<"\":"<<content[4]<<","<<endl;
    In1.getline(temp,MAX_CHARACTER);
    break_line(temp,content);
    json_o<<"\""<<content[0]<<"_"<<content[1]<<"_"<<content[2]<<"\":"<<content[4]<<","<<endl;
    In1.getline(temp,MAX_CHARACTER);
    break_line(temp,content);
    json_o<<"\""<<content[0]<<"_"<<content[1]<<"_"<<content[2]<<"\":"<<content[4]<<","<<endl;
    In1.getline(temp,MAX_CHARACTER);
    break_line(temp,content);
    json_o<<"\""<<content[0]<<"_"<<content[1]<<"_"<<content[2]<<"\":"<<content[4]<<","<<endl;

    pattern="UHF SPIN CONTAMINATION"; 
    int spin_pos=find_pattern(In1,pattern);
    if (spin_pos!=NULL){
      int s2_pos=find_pattern(In1,"Expectation value of");
      In1.seekg(s2_pos);
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"S2\":"<<content[5]<<","<<endl;
    }

    pattern="Summary of Natural Population Analysis";
    int nbo_pos=find_pattern(In1,pattern);
    if (nbo_pos!=NULL){
      char nbo_q[100000],nbo_s[100000];
      strcpy(nbo_q,"\"nbo_charge\":[");
      strcpy(nbo_s,"\"nbo_spin\":[");
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      for (int i=0;i<atomn;i++){
        In1.getline(temp,MAX_CHARACTER);
        break_line(temp,content);
        if (i>0) {
          strcat(nbo_q,", ");
          strcat(nbo_s,", ");
        }
        strcat(nbo_q,content[2].c_str());
        strcat(nbo_s,content[7].c_str());
      }
      json_o<<nbo_q<<"],"<<endl;
      json_o<<nbo_s<<"],"<<endl;
    }

    pattern="DFT-D V3";
    json_o<<std::fixed;
    int d3_pos=find_pattern(In1,pattern);
    if (d3_pos!=NULL){
      d3_pos=find_pattern(In1,"Edisp/kcal");
      In1.seekg(d3_pos);
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"edisp\":"<<atof(content[2].c_str())*Eh2eV<<","<<endl;
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"e6\":"<<atof(content[3].c_str())*kcal2eV<<","<<endl;
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"e8\":"<<atof(content[3].c_str())*kcal2eV<<","<<endl;
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"dispersion_corr\":"<<atof(content[2].c_str())*Eh2eV<<","<<endl;
    }

    pattern="FINAL SINGLE POINT ENERGY";
    json_o<<std::fixed;
    int fe_pos=find_pattern(In1,pattern);
    if (fe_pos!=NULL){
      In1.seekg(fe_pos);
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"energy\":"<<atof(content[4].c_str())*Eh2eV<<","<<endl;
    }

    pattern="CARTESIAN GRADIENT";
    c_pos=find_pattern(In1,pattern);
    if (c_pos!=NULL){
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      column=break_line(temp,content);
      int catomn=0;
      json_o<<"\"forces\":[";
      while (column==6){
        if (catomn>0){
          json_o<<", ";
        }
        catomn++;
        json_o<<"["<<atof(content[3].c_str())*Ehau2eVa<<","<<atof(content[4].c_str())*Ehau2eVa<<","<<atof(content[5].c_str())*Ehau2eVa<<"]";
        In1.getline(temp,MAX_CHARACTER);
        column=break_line(temp,content);
      }
      json_o<<"],"<<endl;
    }

    pattern="DIPOLE MOMENT";
    int dip_pos=find_pattern(In1,pattern);
    if (dip_pos!=NULL){
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"dipole_elec\":["<<atof(content[2].c_str())*bohr2a<<","<<atof(content[3].c_str())*bohr2a<<","<<atof(content[4].c_str())*bohr2a<<"],"<<endl;
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"dipole_nuc\":["<<atof(content[2].c_str())*bohr2a<<","<<atof(content[3].c_str())*bohr2a<<","<<atof(content[4].c_str())*bohr2a<<"],"<<endl;
      In1.getline(temp,MAX_CHARACTER);
      In1.getline(temp,MAX_CHARACTER);
      break_line(temp,content);
      json_o<<"\"dipole\":["<<atof(content[2].c_str())*bohr2a<<","<<atof(content[3].c_str())*bohr2a<<","<<atof(content[4].c_str())*bohr2a<<"]"<<endl;
    }

    json_o<<"}"<<endl;
}

