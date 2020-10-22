//this code is used to analyse 
//output of ORCA. 
//author: Lixin Sun nw13mifaso@gmail.com
#include <map>
#include <stdlib.h>
#include <string>
using namespace std;

#ifndef ATOMIC_MASS
#define ATOMIC_MASS
struct AUM2kg {
    static map<string,double> create_map() {
        map<string,double> m;
        m["H"] = 1.008;
        m["He"] = 4.003;
        m["Li"] = 6.941;
        m["Be"] = 9.012;
        m["B"] = 10.811;
        m["C"] = 12.011;
        m["N"] = 14.007;
        m["O"] = 15.999;
        m["F"] = 18.998;
        m["Ne"] = 20.180;
        m["Na"] = 22.990;
        m["Mg"] = 24.305;
        m["Al"] = 26.982;
        m["Si"] = 28.086;
        m["P"] = 30.974;
        m["S"] = 32.065;
        m["Cl"] = 35.453;
        m["Ar"] = 39.948;
        m["K"] = 39.098;
        m["Ca"] = 40.078;
        m["Sc"] = 44.956;
        m["Ti"] = 47.867;
        m["V"] = 50.942;
        m["Cr"] = 51.996;
        m["Mn"] = 54.938;
        m["Fe"] = 55.845;
        m["Co"] = 58.933;
        m["Ni"] = 58.693;
        m["Cu"] = 63.546;
        m["Zn"] = 65.390;
        m["Ga"] = 69.723;
        m["Ge"] = 72.640;
        m["As"] = 74.922;
        m["Se"] = 78.960;
        m["Br"] = 79.904;
        m["Kr"] = 83.800;
        m["Rb"] = 85.468;
        m["Sr"] = 87.620;
        m["Y"] = 88.906;
        m["Zr"] = 91.224;
        m["Nb"] = 92.906;
        m["Mo"] = 95.940;
        m["Tc"] = 98.000;
        m["Ru"] = 101.070;
        m["Rh"] = 102.906;
        m["Pd"] = 106.420;
        m["Ag"] = 107.868;
        m["Cd"] = 112.411;
        m["In"] = 114.818;
        m["Sn"] = 118.710;
        m["Sb"] = 121.760;
        m["Te"] = 127.600;
        m["I"] = 126.905;
        m["Xe"] = 131.293;
        m["Cs"] = 132.906;
        m["Ba"] = 137.327;
        m["La"] = 138.906;
        m["Ce"] = 140.116;
        m["Pr"] = 140.908;
        m["Nd"] = 144.240;
        m["Pm"] = 145.000;
        m["Sm"] = 150.360;
        m["Eu"] = 151.964;
        m["Gd"] = 157.250;
        m["Tb"] = 158.925;
        m["Dy"] = 162.500;
        m["Ho"] = 164.930;
        m["Er"] = 167.259;
        m["Tm"] = 168.934;
        m["Yb"] = 173.040;
        m["Lu"] = 174.967;
        m["Hf"] = 178.490;
        m["Ta"] = 180.948;
        m["W"] = 183.840;
        m["Re"] = 186.207;
        m["Os"] = 190.230;
        m["Ir"] = 192.217;
        m["Pt"] = 195.078;
        m["Au"] = 196.967;
        m["Hg"] = 200.590;
        m["Tl"] = 204.383;
        m["Pb"] = 207.200;
        m["Bi"] = 208.980;
        m["Po"] = 209.000;
        m["At"] = 210.000;
        m["Rn"] = 222.000;
        m["Fr"] = 223.000;
        m["Ra"] = 226.000;
        m["Ac"] = 227.000;
        m["Th"] = 232.038;
        m["Pa"] = 231.036;
        m["U"] = 238.029;
        m["Np"] = 237.000;
        m["Pu"] = 244.000;
        m["Am"] = 243.000;
        m["Cm"] = 247.000;
        m["Bk"] = 247.000;
        m["Cf"] = 251.000;
        m["Es"] = 252.000;
        m["Fm"] = 257.000;
        m["Md"] = 258.000;
        m["No"] = 259.000;
        m["Lr"] = 262.000;
        m["Rf"] = 261.000;
        m["Db"] = 262.000;
        m["Sg"] = 266.000;
        m["Bh"] = 264.000;
        m["Hs"] = 277.000;
        m["Mt"] = 268.000;
        return m;
    }
    static const map<string,double> atomic_mass;
};
#endif
