/* this is a CMake loadable command to wrap vtk objects into Java */

#include "cmCPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct 
{
  char *LibraryName;
  int NumberWrapped;
  void **SourceFiles;
} cmVTKWrapJavaData;

/* do almost everything in the initial pass */
static int InitialPass(void *inf, void *mf, int argc, char *argv[])
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  int i;
  int newArgc;
  char **newArgv;
  int numWrapped = 0;
  cmVTKWrapJavaData *cdata = 
    (cmVTKWrapJavaData *)malloc(sizeof(cmVTKWrapJavaData));
  const char *cdir = info->CAPI->GetCurrentDirectory(mf);
  const char *def = 0;
  int sourceListSize = 0;
  char *sourceListValue = 0;
  char *newName;
  void *cfile = 0;

  const char* resultDirectory = "${VTK_JAVA_HOME}";
  const char* res = info->CAPI->GetCurrentOutputDirectory(mf);
  char* depFileName;
  FILE* fp;

  if(argc < 3 )
    {
    info->CAPI->SetError(info, "called with incorrect number of arguments");
    return 0;
    }
  
  info->CAPI->ExpandSourceListArguments(mf, argc, (const char**)argv, 
                                        &newArgc, (char***)&newArgv, 2);
  
  /* Now check and see if the value has been stored in the cache */
  /* already, if so use that value and don't look for the program */
  if(!info->CAPI->IsOn(mf,"VTK_WRAP_JAVA"))
    {
    info->CAPI->FreeArguments(newArgc, newArgv);
    return 1;
    }

  /* keep the library name */
  cdata->LibraryName = strdup(newArgv[0]);
  cdata->SourceFiles = (void **)malloc(sizeof(void *)*newArgc);

  /* was the list already populated */
  def = info->CAPI->GetDefinition(mf, newArgv[1]);

  /* Calculate size of source list.  */
  /* Start with list of source files.  */
  sourceListSize = info->CAPI->GetTotalArgumentSize(newArgc,newArgv);
  /* Add enough to extend the name of each class. */
  sourceListSize += newArgc*strlen("Java.cxx");
  /* Add enough to include the def.  */
  sourceListSize += def?strlen(def):0;

  /* Allocate and initialize the source list.  */
  sourceListValue = (char *)malloc(sourceListSize);
  sourceListValue[0] = 0;
  if (def)
    {
    sprintf(sourceListValue,"%s",def);
    }

  /* Prepare java dependency file */
  depFileName = (char*)malloc(strlen(res) + 40);
  sprintf(depFileName, "%s/JavaDependencies.cmake", res);
  fp = fopen(depFileName, "w");
  fprintf(fp, "# This file is automatically generated by CMake VTK_WRAP_JAVA\n\n"
          "SET(VTK_JAVA_DEPENDENCIES ${VTK_JAVA_DEPENDENCIES}\n");

  /* get the classes for this lib */
  for(i = 2; i < newArgc; ++i)
    {   
    void *curr = info->CAPI->GetSource(mf,newArgv[i]);
    
    /* if we should wrap the class */
    if (!curr || 
        !info->CAPI->SourceFileGetPropertyAsBool(curr,"WRAP_EXCLUDE"))
      {
      void *file = info->CAPI->CreateSourceFile();
      char *srcName;
      char *hname=0;
      srcName = info->CAPI->GetFilenameWithoutExtension(newArgv[i]);
      if (curr)
        {
        int abst = info->CAPI->SourceFileGetPropertyAsBool(curr,"ABSTRACT");
        info->CAPI->SourceFileSetProperty(file,"ABSTRACT",
                                          (abst ? "1" : "0"));
        }
      newName = (char *)malloc(strlen(srcName)+5);
      sprintf(newName,"%sJava",srcName);
      info->CAPI->SourceFileSetName2(file, newName, 
                                     info->CAPI->GetCurrentOutputDirectory(mf),
                                     "cxx",0);
      hname = (char *)malloc(strlen(cdir) + strlen(srcName) + 4);
      sprintf(hname,"%s/%s.h",cdir,srcName);
      /* add starting depends */
      info->CAPI->SourceFileAddDepend(file,hname);
      info->CAPI->AddSource(mf,file);
      free(hname);
      cdata->SourceFiles[numWrapped] = file;
      numWrapped++;
      strcat(sourceListValue,";");
      strcat(sourceListValue,newName);
      strcat(sourceListValue,".cxx");        

      /* Write file to java dependency file */
      fprintf(fp, "  %s/%s.java\n", resultDirectory, srcName);
      free(newName);
      info->CAPI->Free(srcName);
      }
    }

  /* Finalize java dependency file */
  fprintf(fp, ")\n");
  fclose(fp);
  
  cdata->NumberWrapped = numWrapped;
  info->CAPI->SetClientData(info,cdata);

  info->CAPI->AddDefinition(mf, newArgv[1], sourceListValue);
  info->CAPI->FreeArguments(newArgc, newArgv);
  free(sourceListValue);
  return 1;
}
  
  
static void FinalPass(void *inf, void *mf) 
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  /* get our client data from initial pass */
  cmVTKWrapJavaData *cdata = 
    (cmVTKWrapJavaData *)info->CAPI->GetClientData(info);
  
  /* first we add the rules for all the .h to Java.cxx files */
  const char *wjava = "${VTK_WRAP_JAVA_EXE}";
  const char *pjava = "${VTK_PARSE_JAVA_EXE}";
  const char *hints = info->CAPI->GetDefinition(mf,"VTK_WRAP_HINTS");
  const char *args[4];
  const char *depends[2];
  const char *depends2[2];
  char **alldeps = 0;
  char* util=0;
  int i;
  int numDepends, numArgs;
  const char *cdir = info->CAPI->GetCurrentDirectory(mf);
  const char *resultDirectory = "${VTK_JAVA_HOME}";

  /* If the first pass terminated early, we have nothing to do.  */
  if(!cdata)
    {
    return;
    }  
  
  /* wrap all the .h files */
  depends[0] = wjava;
  depends2[0] = pjava;
  numDepends = 1;
  if (hints)
    {
    depends[1] = hints;
    depends2[1] = hints;
    numDepends++;
    }
  alldeps = (char**)malloc(sizeof(const char*)*cdata->NumberWrapped);
  for(i = 0; i < cdata->NumberWrapped; i++)
    {
    char *res;
    const char *srcName = info->CAPI->SourceFileGetSourceName(cdata->SourceFiles[i]);
    char *hname = (char *)malloc(strlen(cdir) + strlen(srcName) + 4);
    sprintf(hname,"%s/%s",cdir,srcName);
    hname[strlen(hname)-4]= '\0';
    strcat(hname,".h");
    args[0] = hname;
    numArgs = 1;
    if (hints)
      {
      args[1] = hints;
      numArgs++;
      }
    args[numArgs] = 
      (info->CAPI->SourceFileGetPropertyAsBool(cdata->SourceFiles[i],"ABSTRACT") ?"0" :"1");
    numArgs++;
    res = (char *)malloc(strlen(info->CAPI->GetCurrentOutputDirectory(mf)) + 
                         strlen(srcName) + 6);
    sprintf(res,"%s/%s.cxx",info->CAPI->GetCurrentOutputDirectory(mf),srcName);
    args[numArgs] = res;
    numArgs++;
    info->CAPI->AddCustomCommand(mf, args[0],
                                 wjava, numArgs, (const char**)args, numDepends, (const char**)depends, 
                                 1, (const char**)&res, cdata->LibraryName);
    free(res);

    res = (char *)malloc(strlen(resultDirectory) + 
                         strlen(srcName) + 3);
    sprintf(res,"%s/%s",resultDirectory,srcName);
    sprintf(res+strlen(res)-4, ".java");
    args[numArgs-1] = res;
    info->CAPI->AddCustomCommand(mf, args[0],
                                 pjava, numArgs, args, numDepends, depends2, 
                                 1, (const char**)&res, cdata->LibraryName);
    alldeps[i] = res;
    free(hname);
    }

  util = malloc(strlen(cdata->LibraryName) + 12);
  sprintf(util, "%sJavaClasses", cdata->LibraryName);
  info->CAPI->AddUtilityCommand(mf, util, "", "", 1,
                                cdata->NumberWrapped,
                                (const char**)alldeps,
                                0, 0);  
  for(i = 0; i < cdata->NumberWrapped; i++)
    {
    free(alldeps[i]);
    }
  free(alldeps);
  free(util);
}

static void Destructor(void *inf) 
{
  int i;
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  /* get our client data from initial pass */
  cmVTKWrapJavaData *cdata = 
    (cmVTKWrapJavaData *)info->CAPI->GetClientData(info);
  if (cdata)
    {
    for (i = 0; i < cdata->NumberWrapped; ++i)
      {              
      info->CAPI->DestroySourceFile(cdata->SourceFiles[i]);
      }
    free(cdata->SourceFiles);
    free(cdata->LibraryName);
    free(cdata);
    }
}

static const char* GetTerseDocumentation() 
{
  return "Create Java Wrappers.";
}

static const char* GetFullDocumentation()
{
  return
    "VTK_WRAP_JAVA(resultingLibraryName SourceListName SourceLists ...)";
}

void CM_PLUGIN_EXPORT VTK_WRAP_JAVA2Init(cmLoadedCommandInfo *info)
{
  info->InitialPass = InitialPass;
  info->FinalPass = FinalPass;
  info->Destructor = Destructor;
  info->GetTerseDocumentation = GetTerseDocumentation;
  info->GetFullDocumentation = GetFullDocumentation;  
  info->m_Inherited = 0;
  info->Name = "VTK_WRAP_JAVA2";
}
