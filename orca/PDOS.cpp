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
#define MAX_ENERGYLINE 1000

//set up the final plotting region
//the fermi level is shfited to zero
#define EMIN -30
#define EMAX 15

//set up the buffer size for reading
#define MAX_CHARACTER 1000
#define MAX_COLUMN 20

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

int main(int argc, char **argv){

    char temp[MAX_CHARACTER], * pch,content[MAX_COLUMN][MAX_CHARACTER];
    int column, line=0;
    bool Print_MO=false;
    
    //locate the beginning of the print_MO
    ifstream In1(argv[1]);
    while (In1.good()){
        line++;
        In1.getline(temp,MAX_CHARACTER); 

        column=0;
        pch = strtok (temp," ");
        while ((pch != NULL)&&(column<MAX_COLUMN)) {
            strcpy(content[column],pch);
            column++;
            pch = strtok (NULL, " ");
        }
        pch= NULL;
        if (column == 2 ){
            if (( strcmp(content[0],"MOLECULAR") == 0 ) && (strcmp(content[1],"ORBITALS") == 0)){
                Print_MO=true;
                break;
            }
        }
    }
    if ((! In1.good()) || (Print_MO==false)){
        cout<<"ERROR: MOLECULAR ORBITALS block not found"<<endl;
        cout<<"please write below lines to your orca input script "<<endl;
        cout<<"\%output"<<endl;
        cout<<"Print[ P_Basis ] 2"<<endl;
        cout<<"Print[ P_MOs ] 1"<<endl;
        cout<<"end"<<endl;
        return 1;
    }
    In1.getline(temp,MAX_CHARACTER); //another buffering line
    line++;

    double *tally=new double [2*MAX_ELEMENT*MAX_M*MAX_ENERGYLINE];
    fill(tally, tally+ 2*MAX_ELEMENT*MAX_M*MAX_ENERGYLINE, 0);
    double energy[2*MAX_ENERGYLINE];
    double occupancy[2*MAX_ENERGYLINE];
    char element[MAX_ELEMENT][10];
    int n_element=0;
    char spdf[MAX_M]={'s','p','d','f','g','h'};
    int n_energy[2]={0,0};
    double efermi=-100000;
    
    //spin up and down
    for (int spin=0;spin<2;spin++){
      int energy_id=0;
      int energy_line=0;
      double *p_energy;
      double *p_occupancy;
      double *p_tally;
      do{
          line++;
          In1.getline(temp,MAX_CHARACTER); 


          column=0;
          pch = strtok (temp," ");
          while (pch != NULL) {
              strcpy(content[column],pch);
              column++;
              pch = strtok (NULL, " ");
          }
          pch= NULL;

          if (contain_alphabet(content[0])==false) energy_line+=1;
          //debug cout<<line<<" "<<column<<" "<<energy_line<<endl;

          //the second line is for orbital energy
          if (energy_line == 2){
            //debug cout<<"second line"<<endl;
            p_energy=energy+spin*MAX_ENERGYLINE+energy_id;
            p_occupancy=occupancy+spin*MAX_ENERGYLINE+energy_id;
            p_tally=tally+spin*MAX_ELEMENT*MAX_M*MAX_ENERGYLINE+energy_id;
            for (int columni=0;columni<column;columni++){
               p_energy[columni]=atof(content[columni])*27.2113834; 
            }
            n_energy[spin]+=column;
            energy_id+=column;
          //it starts with non-space character
          } else if ((energy_line ==0)  && (contain_alphabet(content[0])==true)){
            //debug cout<<"meat line"<<endl;

            //remove non aphebatic character in atom name
            string s(content[0]);
            s.erase(remove_if(s.begin(), s.end(), isdigit1), s.end());
            strcpy(content[0],s.c_str());

            //remove non aphebatic character and 'x' 'y' 'z' in orbital name
            string s1(content[1]);
            s1.erase(remove_if(s1.begin(), s1.end(), isdigit1 ), s1.end());
            s1.erase(remove_if(s1.begin(), s1.end(), isxyz ), s1.end());
            strcpy(content[1],s1.c_str());

            //debug cout<<"   "<<content[0]<<" "<<content[1]<<endl;

            //recognize the element name
            int elementid=-1;
            for (int eid=0;eid<n_element;eid++){
                if (strcmp(content[0],element[eid])==0) elementid=eid;
            }
            if (elementid==-1){
                strcpy(element[n_element],content[0]);
                elementid=n_element;
                n_element++;
            }

            //recognize the orbital component
            int spdf_id=-1;
            for (int sid=0;sid<6;sid++){
                if (content[1][0]==spdf[sid]) spdf_id=sid;
            }
            if (spdf_id==-1){
                cout<<" unknown orbital species "<<content[1]<<endl;
                return(1);
            }

            //debug cout<<"   "<<elementid<<" "<<spdf_id<<endl;

            //sum over 
            int es=elementid*MAX_M*MAX_ENERGYLINE+spdf_id*MAX_ENERGYLINE;
            for (int columni = 2; columni < column; columni++){
                p_tally[es+columni-2]+=abs(atof(content[columni])*atof(content[columni]));
            }
          }else if (energy_line == 4 ) energy_line=0;
          else if (energy_line == 3){
            for (int columni=0;columni<column;columni++){
                p_occupancy[columni]=atof(content[columni]);
                if ((p_occupancy[columni]>0) && (p_energy[columni] > efermi)){
                        efermi=p_energy[columni];
                }
            }
          }

      }while (column!=0);
    }
    cout<<"done reading"<<endl;


    ofstream out(argv[2]);
    // output the raw pdos without smearing
    out<<"# efermi:"<<efermi;
    for (int spin=0; spin<2; spin++){
      for (int j=0;j<n_element;j++){
        for (int k=0;k<MAX_M;k++){
            out<<" "<<element[j]<<"_"<<spdf[k]<<"_"<<spin;
        }
      }
    }
    out<<endl;

    /*
    //output the raw pdos without smearing
    for (int i=0;i<n_energy[0];i++){
        if ((energy[i]-efermi)>=-6 && (energy[i]-efermi) <=10){
            out<<energy[i]-efermi<<" ";
            for (int spin=0; spin<2; spin++){
                for (int j=0;j<n_element;j++){
                    for (int k=0;k<MAX_M;k++){
                        out<<tally[spin*MAX_ELEMENT*MAX_M*MAX_ENERGYLINE+j*MAX_M*MAX_ENERGYLINE+k*MAX_ENERGYLINE+i]<<" ";
                    }
                }
            }
            out<<endl;
        }
    }
    */

    //smearing

    double dE=0.1;
    double sigma=0.05;
    double sigma2=sigma*sigma;

    double base=efermi+EMIN;
    double up=efermi+EMAX;
    int grid=int((up-base)/dE);

    double *smearing=new double [2*n_element*MAX_M*grid];
    fill(smearing, smearing + 2*n_element*MAX_M*grid , 0);

    double * spreadE=new double[grid];
    double x[grid];
    for (int i=0;i<grid;i++) x[i]=base+i*dE;

    for (int spin=0; spin<2; spin++){
      double *smearing_spin=smearing+spin*n_element*MAX_M*grid;
      double *tally_spin=tally+spin*MAX_ELEMENT*MAX_M*MAX_ENERGYLINE;
      double *p_energy=energy+spin*MAX_ENERGYLINE;
      for (int i=0;i<n_energy[spin];i++){
        if ((p_energy[i]-efermi)>=EMIN && (p_energy[i]-efermi) <=EMAX){
          double *tally_energy=tally_spin+i;
          gaussian(grid, x, spreadE,p_energy[i],sigma2);
          for (int j=0;j<n_element;j++){
            double *smearing_element=smearing_spin+j*MAX_M*grid;
            double *tally_element=tally_energy+j*MAX_M*MAX_ENERGYLINE;
            for (int k=0;k<MAX_M;k++){
                double *smearing_m=smearing_element+k*grid;
                double *tally_m=tally_element+k*MAX_ENERGYLINE;
                for (int igrid=0; igrid < grid; igrid++){
                  smearing_m[igrid]+=tally_m[0]*spreadE[igrid];
                }
            }
          }
        }
      }
    }
    cout<<"done smearing"<<endl;

    for (int i=0;i<grid;i++){
        out<<x[i]-efermi<<" ";
        double *smearing_energy=&smearing[i];
        for (int spin=0; spin<2; spin++){
          double *smearing_spin=smearing_energy+spin*n_element*MAX_M*grid;
          for (int j=0;j<n_element;j++){
            double *smearing_element=smearing_spin+j*MAX_M*grid;
            for (int k=0;k<MAX_M;k++){
                out<<smearing_element[k*grid]<<" ";
            }
          }
        }
        out<<endl;
    }
}
