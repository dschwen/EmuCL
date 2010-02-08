// work group SIZE is 4*4*4
// shared mem is 6*6*6

__kernel void flux3d(  __global float *c, __global float *lap, int dir, float dt, __local float * shared, 
                      float EAA, float EAB, float EBB, float FAC1,  float FAC2, float FAC3, float FACTOR1, float GB, float Z )
{
  float lb;

  int xg = get_global_id(0);
  int yg = get_global_id(1);
  int zg = get_global_id(2);

  int xl = get_local_id(0);
  int yl = get_local_id(1);
  int zl = get_local_id(2);

  int a = 1+6+36 + xl + yl*6 + zl*36;
  //int b = a + 6*36;
  int ceaa = a + 1*6*36;
  int cebb = a + 2*6*36;
  int leaa = a + 3*6*36;
  int lebb = a + 4*6*36;

  //
  // copy self
  //
  // needs define: SIZE, SIZE2, SIZE3
  shared[a] = c[ xg + yg*SIZE + zg*SIZE2 + dir*SIZE3 ];
  lb = lap[ xg + yg*SIZE + zg*SIZE2 ];

  // needs define: EAA, EAB, EBB
  shared[ceaa] = shared[a] * (EAB-EAA);
  shared[cebb] = shared[a] * (EBB-EAB);
  shared[leaa] = lb * (EAB-EAA);
  shared[lebb] = lb * (EBB-EAB);

  //
  // copy borders
  //
  if( xl == 0 )
  {
    shared[a-1] = c[ ( (xg-1) & (SIZE-1) ) + yg*SIZE + zg*SIZE2 + dir*SIZE3 ];
    lb = lap[ ( (xg-1) & (SIZE-1) ) + yg*SIZE + zg*SIZE2 ];

    shared[ceaa-1] = shared[a-1] * (EAB-EAA);
    shared[cebb-1] = shared[a-1] * (EBB-EAB);

    shared[leaa-1] = lb * (EAB-EAA);
    shared[lebb-1] = lb * (EBB-EAB);
  }
  if( xl == 3 )
  {
    shared[a+1] = c[ ( (xg+1) & (SIZE-1) ) + yg*SIZE + zg*SIZE2 + dir*SIZE3 ];
    lb = lap[ ( (xg+1) & (SIZE-1) ) + yg*SIZE + zg*SIZE2 ];

    shared[ceaa+1] = shared[a+1] * (EAB-EAA);
    shared[cebb+1] = shared[a+1] * (EBB-EAB);

    shared[leaa+1] = lb * (EAB-EAA);
    shared[lebb+1] = lb * (EBB-EAB);
  }
  if( yl == 0 )
  {
    shared[a-6] = c[ xg + ( (yg-1) & (SIZE-1) )*SIZE + zg*SIZE2 + dir*SIZE3 ];
    lb = lap[ xg + ( (yg-1) & (SIZE-1) )*SIZE + zg*SIZE2 ];

    shared[ceaa-6] = shared[a-6] * (EAB-EAA);
    shared[cebb-6] = shared[a-6] * (EBB-EAB);

    shared[leaa-6] = lb * (EAB-EAA);
    shared[lebb-6] = lb * (EBB-EAB);
  }
  if( yl == 3 )
  {
    shared[a+6] = c[ xg + ( (yg+1) & (SIZE-1) )*SIZE + zg*SIZE2 + dir*SIZE3 ];
    lb = lap[ xg + ( (yg+1) & (SIZE-1) )*SIZE + zg*SIZE2 ];

    shared[ceaa+6] = shared[a+6] * (EAB-EAA);
    shared[cebb+6] = shared[a+6] * (EBB-EAB);

    shared[leaa+6] = lb * (EAB-EAA);
    shared[lebb+6] = lb * (EBB-EAB);
  }
  if( zl == 0 )
  {
    shared[a-36] = c[ xg + yg*SIZE + ( (zg-1) & (SIZE-1) )*SIZE2 + dir*SIZE3 ];
    lb = lap[ xg + yg*SIZE + ( (zg-1) & (SIZE-1) )*SIZE2 ];

    shared[ceaa-36] = shared[a-36] * (EAB-EAA);
    shared[cebb-36] = shared[a-36] * (EBB-EAB);

    shared[leaa-36] = lb * (EAB-EAA);
    shared[lebb-36] = lb * (EBB-EAB);
  }
  if( zl == 3 )
  {
    shared[a+36] = c[ xg + yg*SIZE + ( (zg+1) & (SIZE-1) )*SIZE2 + dir*SIZE3 ];
    lb = lap[ xg + yg*SIZE + ( (zg+1) & (SIZE-1) )*SIZE2 ];

    shared[ceaa+36] = shared[a+36] * (EAB-EAA);
    shared[cebb+36] = shared[a+36] * (EBB-EAB);

    shared[leaa+36] = lb * (EAB-EAA);
    shared[lebb+36] = lb * (EBB-EAB);
  }

  //
  // wait for copying to be finished in every worgroup member
  //
  barrier(CLK_LOCAL_MEM_FENCE);

  float E[12],G[12], J[6];

  // e=-1 w=+1 n=-6 s=+6

  // needs define: FAC1, FAC2, FAC3
  E[0] = FAC1 - FAC2 * ( shared[cebb] + shared[ceaa-1] ) - FAC3 * ( shared[lebb] + shared[leaa-1] );
  E[1] = FAC1 - FAC2 * ( shared[cebb-1] + shared[ceaa] ) - FAC3 * ( shared[lebb-1] + shared[leaa] );

  E[2] = FAC1 - FAC2 * ( shared[cebb] + shared[ceaa+1] ) - FAC3 * ( shared[lebb] + shared[leaa+1] );
  E[3] = FAC1 - FAC2 * ( shared[cebb+1] + shared[ceaa] ) - FAC3 * ( shared[lebb+1] + shared[leaa] );

  E[4] = FAC1 - FAC2 * ( shared[cebb] + shared[ceaa-6] ) - FAC3 * ( shared[lebb] + shared[leaa-6] );
  E[5] = FAC1 - FAC2 * ( shared[cebb-6] + shared[ceaa] ) - FAC3 * ( shared[lebb-6] + shared[leaa] );

  E[6] = FAC1 - FAC2 * ( shared[cebb] + shared[ceaa+6] ) - FAC3 * ( shared[lebb] + shared[leaa+6] );
  E[7] = FAC1 - FAC2 * ( shared[cebb+6] + shared[ceaa] ) - FAC3 * ( shared[lebb+6] + shared[leaa] );

  E[8] = FAC1 - FAC2 * ( shared[cebb] + shared[ceaa-36] ) - FAC3 * ( shared[lebb] + shared[leaa-36] );
  E[9] = FAC1 - FAC2 * ( shared[cebb-36] + shared[ceaa] ) - FAC3 * ( shared[lebb-36] + shared[leaa] );

  E[10] = FAC1 - FAC2 * ( shared[cebb] + shared[ceaa+36] ) - FAC3 * ( shared[lebb] + shared[leaa+36] );
  E[11] = FAC1 - FAC2 * ( shared[cebb+36] + shared[ceaa] ) - FAC3 * ( shared[lebb+36] + shared[leaa] );

  // needs define: GB
  for( int i = 0; i < 12; i++ )
    G[i] = exp(-E[i] ) + GB; // native_exp is broken

  // needs define: Z
  J[0] = Z * ( shared[a]*(1.0-shared[a-1] ) * G[0] - shared[a-1]*(1.0-shared[a] ) * G[1] );
  J[1] = Z * ( shared[a]*(1.0-shared[a+1] ) * G[2] - shared[a+1]*(1.0-shared[a] ) * G[3] );
  J[2] = Z * ( shared[a]*(1.0-shared[a-6] ) * G[4] - shared[a-6]*(1.0-shared[a] ) * G[5] );
  J[3] = Z * ( shared[a]*(1.0-shared[a+6] ) * G[6] - shared[a+6]*(1.0-shared[a] ) * G[7] );
  J[4] = Z * ( shared[a]*(1.0-shared[a-36] ) * G[8] - shared[a-36]*(1.0-shared[a] ) * G[9] );
  J[5] = Z * ( shared[a]*(1.0-shared[a+36] ) * G[10] - shared[a+36]*(1.0-shared[a] ) * G[11] );

  // needs define: FAC1

  //
  // write result to other half of the buffer (somewhere I screwed up the signs...)
  //
  c[ xg + yg*SIZE + zg*SIZE2 + (1-dir)*SIZE3 ] = shared[a] - FACTOR1 * ( J[1]+J[0]+J[3]+J[2]+J[5]+J[4] );

  //
  // wait for copying to be finished in every worgroup member
  //
  barrier(CLK_LOCAL_MEM_FENCE);
}
