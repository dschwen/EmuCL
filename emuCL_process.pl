#!/usr/bin/perl

while(<STDIN>)
{
  if( $stream ne ""  || /^__kernel\s+void\s+/  )
  {
    $stream .= $_;
  }
  else
  {
    print $_;
  }

  if( $stream =~ /\{/ )
  {
    $stream =~ s/\n/ /g;
    $stream =~ /\(\s*(.*)\s*\)/g;
    $args = $1;
    @arglist = split( /\s*,\s*/, $args );

    $stream =~ /\{(.*)$/g;
    $tail = $1;
  
    $stream =~ /^__kernel\s+void\s+([^\s\(]+)/;
    $fname = $1;

    print "void $fname( void **__args )\n{ $tail\n";

    $n = 0;
    foreach( @arglist )
    {
      s/\s*__global\s*//g;
      s/\s*__local\s*//g;
      s/\s\*/* /g;
      while( s/\s\s/ /g ) {};

      /^(.+)\s+([^\s]+)$/;
      $type = $1;
      $vname = $2;

      print "  $_ = *(($type*)__args[$n]);\n";
      $n++;
    }
  
    $stream = "";
  }
}

