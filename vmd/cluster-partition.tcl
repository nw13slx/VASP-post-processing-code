proc ladd L {expr [join $L +]+0} ;

#requirement: type should be element symbol instead of number. point charge should be labelled as "F"
#             the configuration does not have periodic boundary

#argument 1: QM_string definition of the QM zone. can be anything that vmd recognize
#argument 2: active_string. region 3
#argument 3: ecp_group can be "none" or "type Ti"
#argument 4: charge of the ecp
#argument 5: name i.e. "CaO2"
#argument 6: partial or formal charge scheme. either "p" or "f"

#partial charge scheme:
#   region 1: QM atom, with charge 0
#   region 2: potential, use partial charge or formal charge
#
#   compensated charge computed from: region 1 original charge + region 2 current charge
#   delta charge: compensated charge divided by the number of atoms in region 3 and region 4
#
#   region 3: active MM, use partial charge, and partial charge shell
#   region 4: frozen MM, use original charge, no shell

#formal charge scheme:
#   region 1: QM atom, with charge 0
#   region 2: potential, use partial charge or formal charge
#
#   compensated charge computed from: region 1 original charge + region 2 current charge
#   delta charge: compensated charge divided by the number of atoms in region 3 and region 4
#
#   region 3: active MM, each 
#   region 4: frozen MM, use original charge

proc selectQM { QM_string active_string ecp_string ecp_q shell_Q name scheme } {


  set fo_info [ open [format "%s.info" $name] "w"]

  #repeats the argument
  puts [ format "original command: selectQM \"%s\" \"%s\" \"%s\" %s %s %s" $QM_string $active_string $ecp_string $ecp_q $name $scheme]
  puts [ format "selection string: %s " $QM_string ]
  puts $fo_info [ format "original command: selectQM \"%s\" \"%s\" \"%s\" %s %s %s" $QM_string $active_string $ecp_string $ecp_q $name $scheme]
  puts $fo_info [ format "selection string: %s " $QM_string ]
  

  global thickness  
  global formal_pQ  
  global formal_nQ  
  global partial_pQ 
  global partial_nQ 
  global pQ_type
  global nQ_type
  global element

  if { [ string match $scheme "p" ] == 1 } {
    set core_Q [ expr $partial_nQ - $shell_Q ]
    set pQ $partial_pQ
    set nQ $partial_nQ
  } else {
    set core_Q [ expr $formal_nQ - $shell_Q ]
    set pQ $formal_pQ
    set nQ $formal_nQ
  }

  puts [ format "pQ %g nQ %g core_Q %g shell_Q %g" $pQ $nQ $core_Q $shell_Q]
  puts $fo_info [ format "pQ %g nQ %g core_Q %g shell_Q %g" $pQ $nQ $core_Q $shell_Q]
  
  
  puts "get all informatino of the QM group $QM_string"
  set QM [ atomselect top $QM_string ]
  set x_QM [ $QM get {x y z} ]
  set type_QM [ $QM get type ]
  set n_QM [ $QM num ]


  set pot_def [ format "not (%s) and (%s) and within %f of (%s)" $QM_string $ecp_string $thickness $QM_string ] 
  set boundary [ atomselect top $pot_def  ]
  set n_pot [ $boundary num ]
  #for each atom

  if { $n_pot > 0 } {
    set region2 $pot_def
    set x_pot [ $boundary get {x y z} ]
    set type_pot [ $boundary get type ]
    puts "region 2: ecp embedding $n_pot "
    set region3 [ format "(%s) and (not (%s)) and (not (%s))" $active_string $QM_string $pot_def ] 
  } else {
    set boundary [ atomselect top "none" ]
    set region2 "none"
    set region3 [ format "(%s) and (not (%s)) " $active_string $QM_string  ] 
  }
  set MM_active [ atomselect top $region3 ]
  set n_active [ $MM_active num ]
  set x_active [ $MM_active get {x y z} ]
  set type_active [ $MM_active get type ]
  set q_active [ $MM_active get charge ]
  puts "region 3: active $n_active" 


  puts "region 4: frozen"
  set region4 [ format "(not (%s)) and (not (%s))" $QM_string $active_string ] 
  set frozen [ atomselect top $region4 ]
  set n_frozen [ $frozen num ]
  set x_frozen [ $frozen get {x y z} ]
  set type_frozen [ $frozen get type ]
  set q_frozen [ $frozen get charge ]

  puts "output basic statistic"
  puts $fo_info [ format "QM: %g boundary: %g MM_active:%g MM_frozen:%g" $n_QM $n_pot $n_active $n_frozen ]
  puts [ format "QM: %g boundary: %g MM_active:%g MM_frozen:%g" $n_QM $n_pot $n_active $n_frozen ]
  
  puts "compute the excess charge"
  set nQ_QM [ [ atomselect top [ format "(type %s ) and (%s)" $nQ_type $QM_string ] ] num ]
  set pQ_QM [ [ atomselect top [ format "(type %s ) and (%s)" $pQ_type $QM_string ] ] num ]
  set newq [ expr $nQ_QM*$formal_nQ+$pQ_QM*$formal_pQ+$n_pot*$ecp_q ]
  puts [ $QM get charge ]
  set q1 [ ladd [ $QM get charge ] ]
  set q2 [ ladd [ $boundary get charge ] ]
  set original_q [ expr $q1+$q2 ]
  set excessq [ expr $newq-$original_q ]
  set netcharge_QM [ expr $nQ_QM*$formal_nQ+$pQ_QM*$formal_pQ ] 
  set delta   [ expr $excessq/$n_active ]
  set pQd [ expr $pQ-$delta ]
  set nQd [ expr $nQ-$delta ]
  set core_Qd [ expr $core_Q-$delta ]

  puts [ format "QM charge %g" $netcharge_QM ]
  puts [ format "excess charge %g = (%g*%g+%g*%g+%g*%g)-%g-%g" $excessq $nQ_QM $formal_nQ $pQ_QM $formal_pQ $n_pot $ecp_q $q1 $q2 ]
  puts [ format "delta: %g" $delta ]
  puts $fo_info [ format "QM charge %g" $netcharge_QM ]
  puts $fo_info [ format "excess charge %g = (%g*%g+%g*%g+%g*%g)-%g-%g" $excessq $nQ_QM $formal_nQ $pQ_QM $formal_pQ $n_pot $ecp_q $q1 $q2 ]
  puts $fo_info [ format "delta: %g" $delta ]

  ##periodic boundary
  #set fo_periodic [ open [format "%s.chm" $name] "w"]
  
  #non-periodic boundary
  set fo_noshell [ open [format "%s_noshell.chm" $name] "w"]
  #with shell predefined
  set fo_shell [ open [format "%s_shell.chm" $name] "w"]
  #for orca testing
  set fo_QM [ open [format "%s.xyz" $name] "w"]
  set fo_MM [ open [format "%s.pc" $name] "w"]
  #force field pre-relaxation
  set fo_ff [ open [format "%s_ff.chm" $name] "w"]

  #puts $fo_periodic "c_create coords=$name.pun {"
  #puts $fo_periodic "connect ionic\n space_group\n 1 \n cell_constants angstrom"

  puts $fo_noshell "c_create coords=${name}_np.pun {"
  puts $fo_noshell "connect ionic\ncoordinates angstrom"
  puts $fo_shell "c_create coords=${name}_s.pun {"
  puts $fo_shell "connect ionic\ncoordinates angstrom"
  set multi [ expr $netcharge_QM*2+1 ]
  puts $fo_QM [ format "*xyz %.0g %.0g" $netcharge_QM $multi ]
  puts $fo_MM [ expr $n_active+$n_frozen ]
  puts $fo_ff "c_create coords=${name}_ff.pun {"
  puts $fo_ff "connect ionic\ncoordinates angstrom"

  #set lx [lindex $vcell 0 ]
  #set ly [lindex $vcell 1 ]
  #set lz [lindex $vcell 2 ]
  #puts $fo_periodic [ format "%g %g %g 90 90 90\ncoordinates" $lx $ly $lz ]

  puts "output region 1 ..."
  for { set j 0 } { $j < $n_QM } { incr j } {
    set c2 [ lindex $x_QM $j ]
    set x [ lindex $c2 0 ]
    set y [ lindex $c2 1 ]
    set z [ lindex $c2 2 ]
    set ts [ lindex $type_QM $j ]
    #set t [ lindex $type_QM $j ]
    #set ts [ lindex $element $t ]
    #puts $fo_periodic [ format "%s1 %g %g %g 0" $ts [ expr ($x-$cQM1+$lx/2.)/$lx ] [expr ($y-$cQM2+$ly/2.)/$ly] [expr ($z-$cQM3+$lz/2.)/$lz] ]
    puts $fo_noshell [ format "%s1 %g %g %g 0" $ts $x $y $z ]
    puts $fo_shell [ format "%s1 %g %g %g 0" $ts $x $y $z ]
    puts $fo_QM [ format "%s %g %g %g" $ts $x $y $z ]
    if { $ts == $pQ_type } {
      puts $fo_ff [ format "%s1 %g %g %g %g" $ts $x $y $z $pQ ]
    } else {
      puts $fo_ff [ format "%s1 %g %g %g %g" $ts $x $y $z $core_Q ]
    }
  }

  puts "output region 2..."
  for { set j 0 } { $j < $n_pot } { incr j } {
    set c2 [ lindex $x_pot $j ]
    set x [ lindex $c2 0 ]
    set y [ lindex $c2 1 ]
    set z [ lindex $c2 2 ]
    set ts [ lindex $type_pot $j ]
    #set t [ lindex $type_pot $j ]
    #set ts [ lindex $element $t ]
    puts $fo_QM [ format "Np> %g %g %g %g"  $ecp_q $x $y $z ]
    #puts $fo_periodic [ format "%s2 %g %g %g 0" $ts [ expr ($x-$cQM1+$lx/2.)/$lx ] [expr ($y-$cQM2+$ly/2.)/$ly] [expr ($z-$cQM3+$lz/2.)/$lz] ]
    puts $fo_noshell [ format "%s2 %g %g %g 0" $ts $x $y $z ]
    puts $fo_shell [ format "%s2 %g %g %g 0" $ts $x $y $z ]
    puts $fo_ff [ format "%s2 %g %g %g %g" $ts $x $y $z $pQ ]
  }

  puts "output region 3..."
  for { set j 0 } { $j < $n_active } { incr j } {
    set c2 [ lindex $x_active $j ]
    set x [ lindex $c2 0 ]
    set y [ lindex $c2 1 ]
    set z [ lindex $c2 2 ]
    set q [ lindex $q_active $j ]
    set ts [ lindex $type_active $j ]
    #set t [ lindex $type_active $j ]
    #set ts [ lindex $elemnt $t ]
    if { [string match $ts $pQ_type ] == 1 } {
      #puts $fo_periodic [ format "%s3 %g %g %g %g" $ts [ expr ($x-$cQM1+$lx/2.)/$lx ] [expr ($y-$cQM2+$ly/2.)/$ly] [expr ($z-$cQM3+$lz/2.)/$lz] $partial_pQ ]
      puts $fo_noshell [ format "%s3 %g %g %g %g" $ts $x $y $z $pQd ]
      puts $fo_shell [ format "%s3 %g %g %g %g" $ts $x $y $z $pQd ]
      puts $fo_MM [ format "%g %g %g %g" $pQd $x $y $z ]
      puts $fo_ff [ format "%s3 %g %g %g %g" $ts $x $y $z $pQd ]
    } elseif { [ string match $ts "F" ] == 1 } {
      #puts $fo_periodic [ format "%s3 %g %g %g %g" $ts [ expr ($x-$cQM1+$lx/2.)/$lx ] [expr ($y-$cQM2+$ly/2.)/$ly] [expr ($z-$cQM3+$lz/2.)/$lz] $q ]
      #puts $fo_orca [ format "Q  %g %g %g %g" $q  $x $y $z ]
      puts $fo_noshell [ format "%s3 %g %g %g %g" $ts $x $y $z $q ]
      puts $fo_shell [ format "%s3 %g %g %g %g" $ts $x $y $z $q ]
      puts $fo_MM [ format "%g %g %g %g" $q $x $y $z ]
      puts $fo_ff [ format "%s3 %g %g %g %g" $ts $x $y $z $q ]
    } else {
      #puts $fo_periodic [ format "%s3 %g %g %g %g" $ts [ expr ($x-$cQM1+$lx/2.)/$lx ] [expr ($y-$cQM2+$ly/2.)/$ly] [expr ($z-$cQM3+$lz/2.)/$lz] $final_nQ ]
      #puts $fo_orca [ format "Q  %g %g %g %g" $formal_nQ  $x $y $z ]
      puts $fo_noshell [ format "%s3 %g %g %g %g" $ts $x $y $z $nQd ]
      puts $fo_shell [ format "%s3 %g %g %g %g" $ts $x $y $z $core_Qd ]
      puts $fo_MM [ format "%g %g %g %g" $nQd $x $y $z ]
      puts $fo_ff [ format "%s3 %g %g %g %g" $ts $x $y $z $core_Qd ]
    }
  }

  puts "output region 4..."
  for { set j 0 } { $j < $n_frozen } { incr j } {
    set c2 [ lindex $x_frozen $j ]
    set x [ lindex $c2 0 ]
    set y [ lindex $c2 1 ]
    set z [ lindex $c2 2 ]
    set q [ lindex $q_frozen $j ]
    set ts [ lindex $type_frozen $j ]
    #set t [ lindex $type_frozen $j ]
    #set ts [ lindex $element $t ]
    #puts $fo_periodic [ format "%s4 %g %g %g %g" $ts [ expr ($x-$cQM1+$lx/2.)/$lx ] [expr ($y-$cQM2+$ly/2.)/$ly] [expr ($z-$cQM3+$lz/2.)/$lz] $partial_pQ ]
    #puts $fo_orca [ format "Q  %g %g %g %g" $formal_pQ  $x $y $z ]
    puts $fo_noshell [ format "%s4 %g %g %g %g" $ts $x $y $z $q ]
    puts $fo_shell [ format "%s4 %g %g %g %g" $ts $x $y $z $q ]
    puts $fo_MM [ format "%g %g %g %g" $q $x $y $z ]
    puts $fo_ff [ format "%s4 %g %g %g %g" $ts $x $y $z $q ]
  }

  puts $fo_shell "shells"
  puts $fo_ff "shells"

  puts "output region 1 ..."
  for { set j 0 } { $j < $n_QM } { incr j } {
    set c2 [ lindex $x_QM $j ]
    set x [ lindex $c2 0 ]
    set y [ lindex $c2 1 ]
    set z [ lindex $c2 2 ]
    set ts [ lindex $type_QM $j ]
    #set t [ lindex $type_QM $j ]
    #set ts [ lindex $element $t ]
    if { [string match $ts $nQ_type ] == 1 } {
        puts $fo_ff [ format "%s1 %g %g %g %g" $ts $x $y $z $shell_Q ]
    }
  }

  puts "output shell region 3..."
  for { set j 0 } { $j < $n_active } { incr j } {
    set c2 [ lindex $x_active $j ]
    set x [ lindex $c2 0 ]
    set y [ lindex $c2 1 ]
    set z [ lindex $c2 2 ]
    set q [ lindex $q_active $j ]
    set ts [ lindex $type_active $j ]
    #set t [ lindex $type_active $j ]
    #set ts [ lindex $element $t ]
    if { [string match $ts $nQ_type ] == 1 } {
      puts $fo_shell [ format "%s3 %g %g %g %g" $ts $x $y $z $shell_Q ]
      puts $fo_ff [ format "%s3 %g %g %g %g" $ts $x $y $z $shell_Q ]
    }
  }

  puts $fo_noshell "}"
  puts $fo_noshell "write_xyz coords=${name}_np.pun file=${name}_np_chemshell.xyz"
  puts $fo_shell "}"
  puts $fo_shell "write_xyz coords=${name}_s.pun file=${name}_s_chemshell.xyz"
  puts $fo_ff "}"
  puts $fo_ff "write_xyz coords=${name}_ff.pun file=${name}_chemff.xyz"
  puts $fo_QM "*"
  #puts $fo_periodic "}"
  #puts $fo_periodic "write_xyz coords=$name.pun file=${name}_chemshell.xyz"

  close $fo_info
  close $fo_noshell
  close $fo_shell
  close $fo_QM
  close $fo_MM
  close $fo_ff
  #close $fo_periodic


  addbond_group "($QM_string) and (type $pQ_type)" "($QM_string) and (type $nQ_type)" 0 3
  addbond_group "($region2)" "($QM_string) and (type $nQ_type)" 0 3

  #color Type {T} cyan

  mol delrep 0 top

  #mol color Type

  mol representation Points 1.0
  mol material Opaque
  mol selection $region3
  mol addrep top

  mol material Transparent
  mol representation Points 1.0
  mol selection $region4 
  mol addrep top

  mol material Opaque
  mol representation CPK 1.0000 0.000000 32.000000 12.000000
  mol selection "$QM_string"
  mol addrep top

  mol representation CPK 0.000000 0.500000 32.000000 12.000000
  mol selection "($QM_string) or ($region2)"
  mol addrep top

  mol material Transparent
  mol representation CPK 1.00000 0.000000 32.000000 12.000000
  mol selection "$region2"
  #mol color colorid 16
  mol addrep top
}
  
#}

