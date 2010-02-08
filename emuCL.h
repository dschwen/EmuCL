
typedef char* cl_program; // the name of the temporary source file
typedef int cl_int;

cl_program clCreateProgramWithSource( cl_context context, cl_int dummy1, (const char**) program_source, void* dummy2, cl_int *err)
{

  *err = 0;
}

cl_int clBuildProgram( cl_program program, cl_int dummy1, void* dummy2, char* options, void* dummy3, void* dummy3 )
{

  return 0;
}

