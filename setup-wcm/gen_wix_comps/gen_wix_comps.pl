#!/usr/bin/perl -n

#     gen_wix_comps.pl
#
#     Разработчики:
#                   Артём Н. Алексеев <aralex@inze.net>
#--------------------------------------------------------------------

BEGIN{
  ($NProd, $NGroup, $TplFile, $Path) = @ARGV;
  @ARGV = ();
  
  open(F, "<$TplFile");
  my @Lines = <F>;
  close(F);
  
  $Tpl = join('', @Lines);
  $Prod = sprintf("%02X", $NProd);
  $Group = sprintf("%02X", $NGroup);
  $NLine = 0;
}


{
  chomp;  
  next if($_ eq '');
  
  my $s = $Tpl;
  my $N = sprintf("%02X", $NLine++);
  my $FileName = $_;  
  
  $s =~ s/pp/$Prod/g;
  $s =~ s/gg/$Group/g;
  $s =~ s/nn/$N/g;
  $s =~ s/PATH/$Path/g;
  $s =~ s/FN/$FileName/g;
  
  print $s;
}
