#!/usr/bin/perl

print "#include <emuCL_kernel.h>\n";

while(<STDIN>)
{
  if( $stream ne ""  || /^__kernel\s+void\s+/  )
  {
    $stream .= $_;
  }
  else
  {
    s/get_(global|local)_(id|size)\s*\(([^\)+])\)/__args->get_$1_$2\[$3\]/g;
    s/barrier\(\s*CLK_LOCAL_MEM_FENCE\s*\)/barrier(__args->fence_barrier)/g;
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

    print "void $fname( void* __vargs )\n{\n";
    print "  __argstruct* __args = (__argstruct*)__vargs;\n";

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

      print "  $_ = *(($type*)(__args->a[$n]));\n";
      $n++;
    }
    print "  $tail\n";
  
    $stream = "";
  }
}

print "\nint main() { return 0; }\n";

