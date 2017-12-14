import numpy as np
DIST=0
DX=1
ANGLE=2
ID=3

class analysis:
  def __init__(self,atom,dos):
      self.atom=atom
      self.dos=dos

  def bandgap(self):
    #try to calculate the band gap
    ef=self.dos.Xenergy
    dos=self.dos
    dos0=dos.dos0
    fermiN=self.dos.fermiN
    nedos=self.dos.nedos

    EGMIN_u=None
    EGMIN_d=None
    EGMAX_u=None
    EGMAX_d=None
    foundEg=0
    finfo = open("band-info", 'w')
    finfo.write('#search between 0 %15.8f %15.8f \n' % (ef[fermiN],ef[fermiN+1]))
    for n in range(fermiN, 0, -1):
       if ((dos0[n,0]!=0) and (EGMIN_u is None)):
          EGMIN_u=ef[n]
       if ((dos0[n,1]!=0) and (EGMIN_d is None)):
          EGMIN_d=ef[n]
    for n in range(fermiN+2, nedos, 1):
       if ((dos0[n,0]!=0) and (EGMAX_u is None)):
          EGMAX_u=ef[n]
       if ((dos0[n,1]!=0) and (EGMAX_d is None)):
          EGMAX_d=ef[n]
    if (EGMIN_u >EGMIN_d):
        EGMIN = EGMIN_u
    else:
        EGMIN = EGMIN_d
    if (EGMAX_u < EGMAX_d):
        EGMAX = EGMAX_u
    else:
        EGMAX = EGMAX_d
    if ((dos0[fermiN,0]!=0) and (dos0[fermiN+1,0]!=0) and (dos0[fermiN+2,0]!=0 )):
        Eg_u=0
    else:
        Eg_u = EGMAX_u-EGMIN_u
    if ((dos0[fermiN,1]!=0) and (dos0[fermiN+1,1]!=0) and (dos0[fermiN+2,1]!=0 )):
        Eg_d=0
    else:
        Eg_d = EGMAX_d-EGMIN_d
    finfo.write('#spin     %15s %15s %15s\n' % ("VBM","CBM","gap"))
    finfo.write('spin_up   %15.8f %15.8f %15.8f\n' % (EGMIN_u,EGMAX_u,Eg_u))
    finfo.write('spin_down %15.8f %15.8f %15.8f\n' % (EGMIN_d,EGMAX_d,Eg_d))
    finfo.write('total     %15.8f %15.8f %15.8f\n' % (EGMIN,EGMAX,EGMAX-EGMIN))
    finfo.close()


  def peak_finder(self,center,span):
    ef=self.dos.Xenergy
    dos=self.dos
    dos0=dos.dos0
    fermiN=self.dos.fermiN
    nedos=self.dos.nedos

    window_d=center-span/2.
    window_u=center+span/2.
    win_down= np.argmin(abs(dos.Xenergy-window_d))
    win_up= np.argmin(abs(dos.Xenergy-window_u))
    win_span=(win_up-win_down)
    ana_dos=dos0[win_down:win_up,0]-dos0[win_down:win_up,1]

    from scipy.signal import find_peaks_cwt
    peak=find_peaks_cwt(ana_dos,np.arange(5,20),min_snr=0.8)
    sigma=(np.square(np.arange(win_span)-peak)).dot(ana_dos)/np.sum(ana_dos)
    sigma=np.sqrt(sigma)
    dx=dos.Xenergy[1]-x0
    #print avg*dx+x0,sigma*dx
    return dos.Xenergy[win_down+peak],sigma*dx

  def peak_weight_center(self,center,span):
    #try to calculate the band gap
    ef=self.dos.Xenergy
    dos=self.dos
    dos0=dos.dos0
    fermiN=self.dos.fermiN
    nedos=self.dos.nedos

    window_d=center-span/2.
    window_u=center+span/2.
    win_down= np.argmin(abs(dos.Xenergy-window_d))
    win_up= np.argmin(abs(dos.Xenergy-window_u))
    win_span=(win_up-win_down)
    ana_dos=dos0[win_down:win_up,0]-dos0[win_down:win_up,1]

    #assuming there is only one gaussian peak
    avg=(np.arange(win_span).dot(ana_dos))/np.sum(ana_dos)
    sigma=(np.square(np.arange(win_span)-avg)).dot(ana_dos)/np.sum(ana_dos)
    sigma=np.sqrt(sigma)

    x0=dos.Xenergy[0]
    dx=dos.Xenergy[1]-x0
    return dos.Xenergy[int(win_down+avg)],sigma*dx

  def find_atomic_plane(self,positions,axis):
    #bin all positions towrad one axis
    if (axis=='x'):
      direction=0
    elif (axis=='y'):
      direction=1
    elif (axis=='z'):
      direction=2
    else:
      print 'please input x, y, or z'
      return None,None,None

    natom=len(positions)
    #get the seperation part for each plane
    dz=0.1
    zmin=np.min(positions[:,direction])-3
    zmax=np.max(positions[:,direction])+3
    binz=np.zeros(int(np.ceil((zmax-zmin)/dz)))
    for i in range(natom):
      z=positions[i,direction]
      idz=int(np.floor((z-zmin)/dz))
      binz[idz]+=1
    from scipy.signal import find_peaks_cwt
    peak_atoms=find_peaks_cwt(binz,np.arange(2,10),min_snr=0.5)
    print "peak",peak_atoms*dz+zmin
    division=np.zeros(len(peak_atoms))
    nplane=len(division)
    for i in range(len(peak_atoms)-1):
      division[i]=zmin+((peak_atoms[i]+peak_atoms[i+1])/2.)*dz
      print -(peak_atoms[i]-peak_atoms[i+1])*dz
    division[nplane-1]=zmax
    #print nplane
    pid=np.array(range(natom))
    #for each atom, find the plane id
    for i in range(natom):
      z=positions[i,direction]
      planeid=0
      while ((planeid<(nplane-1)) and (z>division[planeid])):
        planeid+=1
      pid[i]=int(planeid)
    symbol=range(nplane)

    return pid, nplane, symbol

#    def locater(array):
#      length=len(array)
#      middle=length/2
#      if array[middle] < a[middle - 1]:
#          #only look at the left 1 ... n/2 - 1
#      elif array[middle] < a[middle + 1]:
#          #then only look at the right n/2 +1 ... n
#      else:
#          #n/2 is a peak
#
  def find_ngh(self,rNN):
    x=self.atom.positions
    b0=self.atom.boundary
    b=np.array([b0[0,0],b0[1,1],b0[2,2]])
    bh=b/2.
    #initialize neighborlist
    self.atom.ngh_list=[]
    for i in range(self.atom.natom):
      self.atom.ngh_list+=[{}]
    ngh_list=self.atom.ngh_list
    #ngh_list=[{}] will lead to a weird problem...
    #build up the neighborlist
    for i in range(self.atom.natom):
      for j in range(i+1,self.atom.natom):
        dr=x[j,:]-x[i,:]
        for k in range(3):
          while (abs(dr[k])>bh[k]):
            if (dr[k]>0):
              dr[k]-=b[k]
            else:
              dr[k]+=b[k]
        dist=np.linalg.norm(dr)
        if ( dist < rNN):
          ngh_list[i][j]=[dist,dr]
          ngh_list[j][i]=[dist,dr]
 
  def compute_nye(self,G,Q_dagger):
    ngh_id=self.atom.ngh_id
    A=np.zeros([self.atom.natom,3,3,3])
    nye_tensor=np.zeros([self.atom.natom,3,3])
    G_dagger=np.zeros([self.atom.natom,3,3])
    for atomi in range(self.atom.natom):
      for i in range(3):
        for m in range(3):
          deltaG_im=np.zeros(len(ngh_id[atomi]))
          for n in range(len(ngh_id[atomi])):
            deltaG_im[n]=G[ngh_id[atomi][n],i,m]
          deltaG_im-=G[atomi,i,m]
          A[atomi,i,m,:]=np.array(Q_dagger[atomi]).dot(deltaG_im)
          for j in range(3):
            for k in range(3):
              nye_tensor[atomi,j,k]-=self.perm_parity([j,i,m])*A[atomi,i,m,k]
      G_dagger[atomi,:,:]=np.linalg.pinv(G[atomi,:,:])
    return nye_tensor,G_dagger
 
  def computeG(self,P,Q):
    Qdagger=np.linalg.pinv(np.array(Q))
    G=np.array(Qdagger.dot(np.array(P)))
    return Qdagger,G
  
  def match_neigh2P(self,neighbor,P0,angles,Qunsort,criteria=0.9,criteria_max=1):
    for j in neighbor.keys():
      if ( type(j) is int):
        compare=np.zeros(len(P0))
        for k in range(len(P0)):
          compare[k]=angles[j,k]
        indexk=np.argmax(compare)
        anglemin=compare[indexk]
        if (anglemin>criteria):
           if (indexk not in Qunsort.keys()): #(Q[ref][indexk]==[]):
             Qunsort[indexk]=[neighbor[j][DIST],neighbor[j][DX],anglemin,j]
             del neighbor[j]
           elif ((Qunsort[indexk][DIST] > neighbor[j][DIST]) and (Qunsort[indexk][ANGLE]<criteria_max)):
             neighbor[Qunsort[indexk][ID]] = [Qunsort[indexk][DIST],Qunsort[indexk][DX]]
             Qunsort[indexk]=[neighbor[j][DIST],neighbor[j][DX],anglemin,j]
             del neighbor[j]
  
  def add_nearest_neigh(self,neighbor,P0,angles,Qunsort,missing_ngh):
    sort_list=sorted(neighbor.iteritems(),key=lambda (k,v):v[0])
    for item in sort_list[:missing_ngh]:
      j=item[0]
      a={}
      for k in range(len(P0)):
         a[j,k]=angles[j,k]
      sort_angle=sorted(a.iteritems(),key=lambda (k,v):-v)
      for angle in sort_angle:
        if ( j in neighbor.keys()):
          indexk=angle[0][1]
          if (indexk not in Qunsort.keys()): #(Q[ref][indexk]==[]):
            Qunsort[indexk]=[neighbor[j][DIST],neighbor[j][DX],angle[1],j]
            del neighbor[j]
    del sort_list
  
  def perm_parity(self,lst):
    parity = 1
    for i in range(0,len(lst)-1):
        if lst[i] != i:
            parity *= -1
            mn = min(range(i,len(lst)), key=lst.__getitem__)
            lst[i],lst[mn] = lst[mn],lst[i]
    return parity   
  
  def computeQ(self,P_allspecies=None,Pn_allspecies=None):
    ngh_list=self.atom.ngh_list
    if ((P_allspecies is not None ) and (Pn_allspecies is not None)):
      #sort the neighbor and compute Q and G for each atom
      Q=[]
      Q_dagger=[]
      G=np.zeros([self.atom.natom,3,3])
      self.atom.ngh_id=[]
      ngh_id=self.atom.ngh_id
      for i in range(self.atom.natom):
        Q+=[[]]
        Q_dagger+=[[]]
        ngh_id+=[[]]
        Pn=Pn_allspecies[self.atom.species[i]]
        if ( Pn is None):
          print "the reference input is not intact, please check Pn[",self.atom.species[i],"]"
          return None, None,None
        least_missing_id=-1
        least_missing_value=np.max([len(Pn[ref][:,0]) for ref in range(len(Pn))])
        angle_store=[]
        Qunsort=[] 
  
        #first try to find out which pattern matches the best"
        for ref in range(len(Pn)):
          angle_store+=[{}]
          Qunsort+=[{}] 
          for j in ngh_list[i].keys():
            for k in range(len(Pn[ref])):
               angle_store[ref][j,k]=np.array(ngh_list[i][j][DX]).dot(Pn[ref][k,:])/ngh_list[i][j][DIST]
          self.match_neigh2P(ngh_list[i],Pn[ref],angle_store[ref],Qunsort[ref])
          missing_ngh=len(Pn[ref])-len(Qunsort[ref])
          if (least_missing_value > missing_ngh):
            least_missing_value=missing_ngh
            least_missing_id=ref
        if (len(Pn)==2):
          if ((len(Pn[0])-len(Qunsort[0]))==(len(Pn[1])-len(Qunsort[1]))):
            print "competing",self.atom.positions[i]
  
        ref=least_missing_id
        #sort the rest,start from the nearest remaining one
        criteria=0.9
        while ((missing_ngh >0) and (criteria>0)):
          self.match_neigh2P(ngh_list[i],Pn[ref],angle_store[ref],Qunsort[ref],criteria,criteria+0.1)
          missing_ngh=len(Pn[ref])-len(Qunsort[ref])
          criteria-=0.05
  
        if (missing_ngh > len(ngh_list[i])):
           print "need a larger neighbor list"
           print "not enough neighbor to build Q"
           return None,None,None
        elif (missing_ngh >0):
           self.add_nearest_neigh(ngh_list[i],Pn[ref],angle_store[ref],Qunsort[ref],missing_ngh)
  
        P=P_allspecies[self.atom.species[i]][ref]
        if ( P is None):
          print "the reference input is not intact, please check Pn[",self.atom.species[i],"]"
          return None, None,None
        Q[i]=np.zeros(P.shape)
        ngh_id[i]=[[]]*len(P)
        for k in range(len(P)):
          Q[i][k,:]=Qunsort[ref][k][DX]
          ngh_id[i][k]=Qunsort[ref][k][ID]
        Q_dagger[i],G[i,:,:]=self.computeG(P,Q[i])
        del angle_store
        del Qunsort
      del Q
      return Q_dagger,G
    else:
      #sort the neighbor and compute Q and G for each atom
      Q=[]
      Q_dagger=[]
      if self.atom.ngh_id is not None:
         del self.atom.ngh_id
      self.atom.ngh_id=[]
      ngh_id=self.atom.ngh_id
      for i in range(self.atom.natom):
        Q+=[np.zeros([len(ngh_list[i]),3])]
        Q_dagger+=[np.zeros([len(ngh_list[i]),3])]
        ngh_id+=[len(ngh_list[i])*[[]]]
        indexk=0
        for j in ngh_list[i].keys():
          Q[i][indexk,:]=ngh_list[i][j][DX]
          ngh_id[i][indexk]=j
          indexk+=1
        Q_dagger[i]=np.linalg.pinv(np.array(Q[i]))
      del Q
      return Q_dagger,None

  def compute_strain_scalar(self,G_dagger):
    volume=np.zeros(len(G_dagger[:,0,0]))
    vonMises=np.zeros(len(G_dagger[:,0,0]))
    for i in range(len(G_dagger)):
      volume[i]=G_dagger[i,0,0]
      volume[i]+=G_dagger[i,1,1]
      volume[i]+=G_dagger[i,2,2]
      volume[i]-=3.
      print i,G_dagger[i][0,0],G_dagger[i][1,1],G_dagger[i][2,2],volume[i]
      F=1/2.*(G_dagger[i,:,:]+G_dagger[i,:,:].T)
      G_temp=F-np.array([[1,0,0],[0,1,0],[0,0,1]])*(volume[i]/3.+1)
      G_temp=G_temp.dot(G_temp)
      vonMises[i]=np.sqrt((G_temp[0,0]+G_temp[1,1]+G_temp[2,2])/2.)
    return volume, vonMises

