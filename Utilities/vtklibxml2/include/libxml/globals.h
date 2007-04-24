/*
 * Summary: interface for all global variables of the library
 * Description: all the global variables and thread handling for
 *              those variables is handled by this module.
 *
 * The bottom of this file is automatically generated by build_glob.py
 * based on the description file global.data
 *
 * Copy: See Copyright for the status of this software.
 *
 * Author: Gary Pennington <Gary.Pennington@uk.sun.com>, Daniel Veillard
 */

#ifndef __XML_GLOBALS_H
#define __XML_GLOBALS_H

#include <libxml/xmlversion.h>
#include <libxml/parser.h>
#include <libxml/xmlerror.h>
#include <libxml/SAX.h>
#include <libxml/SAX2.h>
#include <libxml/xmlmemory.h>

#ifdef __cplusplus
extern "C" {
#endif

XMLPUBFUN void XMLCALL xmlInitGlobals(void);
XMLPUBFUN void XMLCALL xmlCleanupGlobals(void);

typedef xmlParserInputBufferPtr (*xmlParserInputBufferCreateFilenameFunc) (const char *URI, xmlCharEncoding enc);
typedef xmlOutputBufferPtr (*xmlOutputBufferCreateFilenameFunc) (const char *URI, xmlCharEncodingHandlerPtr encoder, int compression);
XMLPUBFUN xmlParserInputBufferCreateFilenameFunc
XMLCALL xmlParserInputBufferCreateFilenameDefault (xmlParserInputBufferCreateFilenameFunc func);
XMLPUBFUN xmlOutputBufferCreateFilenameFunc
XMLCALL xmlOutputBufferCreateFilenameDefault (xmlOutputBufferCreateFilenameFunc func);

/*
 * Externally global symbols which need to be protected for backwards
 * compatibility support.
 */

#undef  docbDefaultSAXHandler
#undef  htmlDefaultSAXHandler
#undef  oldXMLWDcompatibility
#undef  xmlBufferAllocScheme
#undef  xmlDefaultBufferSize
#undef  xmlDefaultSAXHandler
#undef  xmlDefaultSAXLocator
#undef  xmlDoValidityCheckingDefaultValue
#undef  xmlFree
#undef  xmlGenericError
#undef  xmlStructuredError
#undef  xmlGenericErrorContext
#undef  xmlGetWarningsDefaultValue
#undef  xmlIndentTreeOutput
#undef  xmlTreeIndentString
#undef  xmlKeepBlanksDefaultValue
#undef  xmlLineNumbersDefaultValue
#undef  xmlLoadExtDtdDefaultValue
#undef  xmlMalloc
#undef  xmlMallocAtomic
#undef  xmlMemStrdup
#undef  xmlParserDebugEntities
#undef  xmlParserVersion
#undef  xmlPedanticParserDefaultValue
#undef  xmlRealloc
#undef  xmlSaveNoEmptyTags
#undef  xmlSubstituteEntitiesDefaultValue
#undef  xmlRegisterNodeDefaultValue
#undef  xmlDeregisterNodeDefaultValue
#undef  xmlLastError
#undef  xmlParserInputBufferCreateFilenameValue
#undef  xmlOutputBufferCreateFilenameValue

typedef void (*xmlRegisterNodeFunc) (xmlNodePtr node);
typedef void (*xmlDeregisterNodeFunc) (xmlNodePtr node);

typedef struct _xmlGlobalState xmlGlobalState;
typedef xmlGlobalState *xmlGlobalStatePtr;
struct _xmlGlobalState 
{
        const char *xmlParserVersion;

        xmlSAXLocator xmlDefaultSAXLocator;
        xmlSAXHandlerV1 xmlDefaultSAXHandler;
        xmlSAXHandlerV1 docbDefaultSAXHandler;
        xmlSAXHandlerV1 htmlDefaultSAXHandler;

        xmlFreeFunc xmlFree;
        xmlMallocFunc xmlMalloc;
        xmlStrdupFunc xmlMemStrdup;
        xmlReallocFunc xmlRealloc;

        xmlGenericErrorFunc xmlGenericError;
        xmlStructuredErrorFunc xmlStructuredError;
        void *xmlGenericErrorContext;

        int oldXMLWDcompatibility;

        xmlBufferAllocationScheme xmlBufferAllocScheme;
        int xmlDefaultBufferSize;

        int xmlSubstituteEntitiesDefaultValue;
        int xmlDoValidityCheckingDefaultValue;
        int xmlGetWarningsDefaultValue;
        int xmlKeepBlanksDefaultValue;
        int xmlLineNumbersDefaultValue;
        int xmlLoadExtDtdDefaultValue;
        int xmlParserDebugEntities;
        int xmlPedanticParserDefaultValue;

        int xmlSaveNoEmptyTags;
        int xmlIndentTreeOutput;
        const char *xmlTreeIndentString;

        xmlRegisterNodeFunc xmlRegisterNodeDefaultValue;
        xmlDeregisterNodeFunc xmlDeregisterNodeDefaultValue;

        xmlMallocFunc xmlMallocAtomic;
        xmlError xmlLastError;

        xmlParserInputBufferCreateFilenameFunc xmlParserInputBufferCreateFilenameValue;
        xmlOutputBufferCreateFilenameFunc xmlOutputBufferCreateFilenameValue;
};

#ifdef __cplusplus
}
#endif
#include <libxml/threads.h>
#ifdef __cplusplus
extern "C" {
#endif

XMLPUBFUN void XMLCALL  xmlInitializeGlobalState(xmlGlobalStatePtr gs);

XMLPUBFUN void XMLCALL xmlThrDefSetGenericErrorFunc(void *ctx, xmlGenericErrorFunc handler);

XMLPUBFUN void XMLCALL xmlThrDefSetStructuredErrorFunc(void *ctx, xmlStructuredErrorFunc handler);

XMLPUBFUN xmlRegisterNodeFunc XMLCALL xmlRegisterNodeDefault(xmlRegisterNodeFunc func);
XMLPUBFUN xmlRegisterNodeFunc XMLCALL xmlThrDefRegisterNodeDefault(xmlRegisterNodeFunc func);
XMLPUBFUN xmlDeregisterNodeFunc XMLCALL xmlDeregisterNodeDefault(xmlDeregisterNodeFunc func);
XMLPUBFUN xmlDeregisterNodeFunc XMLCALL xmlThrDefDeregisterNodeDefault(xmlDeregisterNodeFunc func);

XMLPUBFUN xmlOutputBufferCreateFilenameFunc XMLCALL 
        xmlThrDefOutputBufferCreateFilenameDefault(xmlOutputBufferCreateFilenameFunc func);
XMLPUBFUN xmlParserInputBufferCreateFilenameFunc XMLCALL 
        xmlThrDefParserInputBufferCreateFilenameDefault(xmlParserInputBufferCreateFilenameFunc func);

/** DOC_DISABLE */
/*
 * In general the memory allocation entry points are not kept
 * thread specific but this can be overridden by LIBXML_THREAD_ALLOC_ENABLED
 *    - xmlMalloc
 *    - xmlMallocAtomic
 *    - xmlRealloc
 *    - xmlMemStrdup
 *    - xmlFree
 */

#ifdef LIBXML_THREAD_ALLOC_ENABLED
#ifdef LIBXML_THREAD_ENABLED
XMLPUBFUN  xmlMallocFunc * XMLCALL __xmlMalloc(void);
#define xmlMalloc \
(*(__xmlMalloc()))
#else
XMLPUBVAR xmlMallocFunc xmlMalloc;
#endif

#ifdef LIBXML_THREAD_ENABLED
XMLPUBFUN  xmlMallocFunc * XMLCALL __xmlMallocAtomic(void);
#define xmlMallocAtomic \
(*(__xmlMallocAtomic()))
#else
XMLPUBVAR xmlMallocFunc xmlMallocAtomic;
#endif

#ifdef LIBXML_THREAD_ENABLED
XMLPUBFUN  xmlReallocFunc * XMLCALL __xmlRealloc(void);
#define xmlRealloc \
(*(__xmlRealloc()))
#else
XMLPUBVAR xmlReallocFunc xmlRealloc;
#endif

#ifdef LIBXML_THREAD_ENABLED
XMLPUBFUN  xmlFreeFunc * XMLCALL __xmlFree(void);
#define xmlFree \
(*(__xmlFree()))
#else
XMLPUBVAR xmlFreeFunc xmlFree;
#endif

#ifdef LIBXML_THREAD_ENABLED
XMLPUBFUN  xmlStrdupFunc * XMLCALL __xmlMemStrdup(void);
#define xmlMemStrdup \
(*(__xmlMemStrdup()))
#else
XMLPUBVAR xmlStrdupFunc xmlMemStrdup;
#endif

#else /* !LIBXML_THREAD_ALLOC_ENABLED */
XMLPUBVAR xmlMallocFunc xmlMalloc;
XMLPUBVAR xmlMallocFunc xmlMallocAtomic;
XMLPUBVAR xmlReallocFunc xmlRealloc;
XMLPUBVAR xmlFreeFunc xmlFree;
XMLPUBVAR xmlStrdupFunc xmlMemStrdup;
#endif /* LIBXML_THREAD_ALLOC_ENABLED */

#ifdef LIBXML_DOCB_ENABLED
XMLPUBFUN  xmlSAXHandlerV1 * XMLCALL __docbDefaultSAXHandler(void);
#ifdef LIBXML_THREAD_ENABLED
#define docbDefaultSAXHandler \
(*(__docbDefaultSAXHandler()))
#else
XMLPUBVAR xmlSAXHandlerV1 docbDefaultSAXHandler;
#endif
#endif

#ifdef LIBXML_HTML_ENABLED
XMLPUBFUN xmlSAXHandlerV1 * XMLCALL __htmlDefaultSAXHandler(void);
#ifdef LIBXML_THREAD_ENABLED
#define htmlDefaultSAXHandler \
(*(__htmlDefaultSAXHandler()))
#else
XMLPUBVAR xmlSAXHandlerV1 htmlDefaultSAXHandler;
#endif
#endif

XMLPUBFUN xmlError * XMLCALL __xmlLastError(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlLastError \
(*(__xmlLastError()))
#else
XMLPUBVAR xmlError xmlLastError;
#endif

/*
 * Everything starting from the line below is
 * Automatically generated by build_glob.py.
 * Do not modify the previous line.
 */


XMLPUBFUN int * XMLCALL __oldXMLWDcompatibility(void);
#ifdef LIBXML_THREAD_ENABLED
#define oldXMLWDcompatibility \
(*(__oldXMLWDcompatibility()))
#else
XMLPUBVAR int oldXMLWDcompatibility;
#endif

XMLPUBFUN xmlBufferAllocationScheme * XMLCALL __xmlBufferAllocScheme(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlBufferAllocScheme \
(*(__xmlBufferAllocScheme()))
#else
XMLPUBVAR xmlBufferAllocationScheme xmlBufferAllocScheme;
#endif
XMLPUBFUN xmlBufferAllocationScheme XMLCALL xmlThrDefBufferAllocScheme(xmlBufferAllocationScheme v);

XMLPUBFUN int * XMLCALL __xmlDefaultBufferSize(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlDefaultBufferSize \
(*(__xmlDefaultBufferSize()))
#else
XMLPUBVAR int xmlDefaultBufferSize;
#endif
XMLPUBFUN int XMLCALL xmlThrDefDefaultBufferSize(int v);

XMLPUBFUN xmlSAXHandlerV1 * XMLCALL __xmlDefaultSAXHandler(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlDefaultSAXHandler \
(*(__xmlDefaultSAXHandler()))
#else
XMLPUBVAR xmlSAXHandlerV1 xmlDefaultSAXHandler;
#endif

XMLPUBFUN xmlSAXLocator * XMLCALL __xmlDefaultSAXLocator(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlDefaultSAXLocator \
(*(__xmlDefaultSAXLocator()))
#else
XMLPUBVAR xmlSAXLocator xmlDefaultSAXLocator;
#endif

XMLPUBFUN int * XMLCALL __xmlDoValidityCheckingDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlDoValidityCheckingDefaultValue \
(*(__xmlDoValidityCheckingDefaultValue()))
#else
XMLPUBVAR int xmlDoValidityCheckingDefaultValue;
#endif
XMLPUBFUN int XMLCALL xmlThrDefDoValidityCheckingDefaultValue(int v);

XMLPUBFUN xmlGenericErrorFunc * XMLCALL __xmlGenericError(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlGenericError \
(*(__xmlGenericError()))
#else
XMLPUBVAR xmlGenericErrorFunc xmlGenericError;
#endif

XMLPUBFUN xmlStructuredErrorFunc * XMLCALL __xmlStructuredError(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlStructuredError \
(*(__xmlStructuredError()))
#else
XMLPUBVAR xmlStructuredErrorFunc xmlStructuredError;
#endif

XMLPUBFUN void * * XMLCALL __xmlGenericErrorContext(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlGenericErrorContext \
(*(__xmlGenericErrorContext()))
#else
XMLPUBVAR void * xmlGenericErrorContext;
#endif

XMLPUBFUN int * XMLCALL __xmlGetWarningsDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlGetWarningsDefaultValue \
(*(__xmlGetWarningsDefaultValue()))
#else
XMLPUBVAR int xmlGetWarningsDefaultValue;
#endif
XMLPUBFUN int XMLCALL xmlThrDefGetWarningsDefaultValue(int v);

XMLPUBFUN int * XMLCALL __xmlIndentTreeOutput(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlIndentTreeOutput \
(*(__xmlIndentTreeOutput()))
#else
XMLPUBVAR int xmlIndentTreeOutput;
#endif
XMLPUBFUN int XMLCALL xmlThrDefIndentTreeOutput(int v);

XMLPUBFUN const char * * XMLCALL __xmlTreeIndentString(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlTreeIndentString \
(*(__xmlTreeIndentString()))
#else
XMLPUBVAR const char * xmlTreeIndentString;
#endif
XMLPUBFUN const char * XMLCALL xmlThrDefTreeIndentString(const char * v);

XMLPUBFUN int * XMLCALL __xmlKeepBlanksDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlKeepBlanksDefaultValue \
(*(__xmlKeepBlanksDefaultValue()))
#else
XMLPUBVAR int xmlKeepBlanksDefaultValue;
#endif
XMLPUBFUN int XMLCALL xmlThrDefKeepBlanksDefaultValue(int v);

XMLPUBFUN int * XMLCALL __xmlLineNumbersDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlLineNumbersDefaultValue \
(*(__xmlLineNumbersDefaultValue()))
#else
XMLPUBVAR int xmlLineNumbersDefaultValue;
#endif
XMLPUBFUN int XMLCALL xmlThrDefLineNumbersDefaultValue(int v);

XMLPUBFUN int * XMLCALL __xmlLoadExtDtdDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlLoadExtDtdDefaultValue \
(*(__xmlLoadExtDtdDefaultValue()))
#else
XMLPUBVAR int xmlLoadExtDtdDefaultValue;
#endif
XMLPUBFUN int XMLCALL xmlThrDefLoadExtDtdDefaultValue(int v);

XMLPUBFUN int * XMLCALL __xmlParserDebugEntities(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlParserDebugEntities \
(*(__xmlParserDebugEntities()))
#else
XMLPUBVAR int xmlParserDebugEntities;
#endif
XMLPUBFUN int XMLCALL xmlThrDefParserDebugEntities(int v);

XMLPUBFUN const char * * XMLCALL __xmlParserVersion(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlParserVersion \
(*(__xmlParserVersion()))
#else
XMLPUBVAR const char * xmlParserVersion;
#endif

XMLPUBFUN int * XMLCALL __xmlPedanticParserDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlPedanticParserDefaultValue \
(*(__xmlPedanticParserDefaultValue()))
#else
XMLPUBVAR int xmlPedanticParserDefaultValue;
#endif
XMLPUBFUN int XMLCALL xmlThrDefPedanticParserDefaultValue(int v);

XMLPUBFUN int * XMLCALL __xmlSaveNoEmptyTags(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlSaveNoEmptyTags \
(*(__xmlSaveNoEmptyTags()))
#else
XMLPUBVAR int xmlSaveNoEmptyTags;
#endif
XMLPUBFUN int XMLCALL xmlThrDefSaveNoEmptyTags(int v);

XMLPUBFUN int * XMLCALL __xmlSubstituteEntitiesDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlSubstituteEntitiesDefaultValue \
(*(__xmlSubstituteEntitiesDefaultValue()))
#else
XMLPUBVAR int xmlSubstituteEntitiesDefaultValue;
#endif
XMLPUBFUN int XMLCALL xmlThrDefSubstituteEntitiesDefaultValue(int v);

XMLPUBFUN xmlRegisterNodeFunc * XMLCALL __xmlRegisterNodeDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlRegisterNodeDefaultValue \
(*(__xmlRegisterNodeDefaultValue()))
#else
XMLPUBVAR xmlRegisterNodeFunc xmlRegisterNodeDefaultValue;
#endif

XMLPUBFUN xmlDeregisterNodeFunc * XMLCALL __xmlDeregisterNodeDefaultValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlDeregisterNodeDefaultValue \
(*(__xmlDeregisterNodeDefaultValue()))
#else
XMLPUBVAR xmlDeregisterNodeFunc xmlDeregisterNodeDefaultValue;
#endif

XMLPUBFUN xmlParserInputBufferCreateFilenameFunc * XMLCALL __xmlParserInputBufferCreateFilenameValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlParserInputBufferCreateFilenameValue \
(*(__xmlParserInputBufferCreateFilenameValue()))
#else
XMLPUBVAR xmlParserInputBufferCreateFilenameFunc xmlParserInputBufferCreateFilenameValue;
#endif

XMLPUBFUN xmlOutputBufferCreateFilenameFunc * XMLCALL __xmlOutputBufferCreateFilenameValue(void);
#ifdef LIBXML_THREAD_ENABLED
#define xmlOutputBufferCreateFilenameValue \
(*(__xmlOutputBufferCreateFilenameValue()))
#else
XMLPUBVAR xmlOutputBufferCreateFilenameFunc xmlOutputBufferCreateFilenameValue;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XML_GLOBALS_H */
