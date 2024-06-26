/* WIDE AREA INFORMATION SERVER SOFTWARE:
   Developed by Thinking Machines Corporation and put into the public
   domain with no guarantees or restrictions.
*/

/*----------------------------------------------------------------------*/

#define _C_Type_1_Query_

#include "ztype1.h"
#include "cutil.h"
#include "panic.h"
#include "waislog.h"
#include <string.h>

/*----------------------------------------------------------------------*/

query_term*
makeAttributeTerm(use,
		  relation,
		  position,
		  structure,
		  truncation,
		  completeness,
		  term)
char* use;
char* relation;
char* position;
char* structure;
char* truncation;
char* completeness;
any* term;
{
  query_term* qt = (query_term*)s_malloc((size_t)sizeof(query_term));

  qt->TermType = TT_Attribute;

  
  strncpy(qt->Use,use,ATTRIBUTE_SIZE);
  strncpy(qt->Relation,relation,ATTRIBUTE_SIZE);
  strncpy(qt->Position,position,ATTRIBUTE_SIZE);
  strncpy(qt->Structure,structure,ATTRIBUTE_SIZE);
  strncpy(qt->Truncation,truncation,ATTRIBUTE_SIZE);
  strncpy(qt->Completeness,completeness,ATTRIBUTE_SIZE);

  qt->Term = duplicateAny(term);

  qt->ResultSetID = NULL;

  return(qt);
}

/*----------------------------------------------------------------------*/

query_term*
makeResultSetTerm(resultSet)
any* resultSet;
{ 
  query_term* qt = (query_term*)s_malloc((size_t)sizeof(query_term));

  qt->TermType = TT_ResultSetID;

  qt->ResultSetID = duplicateAny(resultSet);

  qt->Term = NULL;
  
  return(qt);
}

/*----------------------------------------------------------------------*/

query_term* 
makeOperatorTerm(operatorCode)
char* operatorCode;
{
  query_term* qt = (query_term*)s_malloc((size_t)sizeof(query_term));

  qt->TermType = TT_Operator;

  strncpy(qt->Operator,operatorCode,OPERATOR_SIZE);

  qt->Term = NULL;
  qt->ResultSetID = NULL;

  return(qt);
}

/*----------------------------------------------------------------------*/

void 
freeTerm(qt)
query_term* qt;
{
  switch (qt->TermType)
    { case TT_Attribute:
	freeAny(qt->Term);
	break;
      case TT_ResultSetID:
	freeAny(qt->ResultSetID);
	break;
      case TT_Operator:
	
	break;
      default:
	panic("Implementation error: Unknown term type %ld",
	      qt->TermType);
	break;
      }
  s_free(qt);
}

/*----------------------------------------------------------------------*/

#define ATTRIBUTE_LIST_SIZE	ATTRIBUTE_SIZE * 6
#define AT_DELIMITER	" "

char* 
writeQueryTerm(qt,buffer,len)
query_term* qt;
char* buffer;
long* len;
{
  char* buf = buffer;
  char attributes[ATTRIBUTE_LIST_SIZE];

  switch (qt->TermType)
    { case TT_Attribute:
	strncpy(attributes,qt->Use,ATTRIBUTE_LIST_SIZE); 
	s_strncat(attributes,AT_DELIMITER,sizeof(AT_DELIMITER) + 1,ATTRIBUTE_LIST_SIZE);
	s_strncat(attributes,qt->Relation,ATTRIBUTE_SIZE,ATTRIBUTE_LIST_SIZE); 
	s_strncat(attributes,AT_DELIMITER,sizeof(AT_DELIMITER) + 1,ATTRIBUTE_LIST_SIZE);
	s_strncat(attributes,qt->Position,ATTRIBUTE_SIZE,ATTRIBUTE_LIST_SIZE); 
	s_strncat(attributes,AT_DELIMITER,sizeof(AT_DELIMITER) + 1,ATTRIBUTE_LIST_SIZE);
	s_strncat(attributes,qt->Structure,ATTRIBUTE_SIZE,ATTRIBUTE_LIST_SIZE); 
	s_strncat(attributes,AT_DELIMITER,sizeof(AT_DELIMITER) + 1,ATTRIBUTE_LIST_SIZE);
	s_strncat(attributes,qt->Truncation,ATTRIBUTE_SIZE,ATTRIBUTE_LIST_SIZE); 
	s_strncat(attributes,AT_DELIMITER,sizeof(AT_DELIMITER) + 1,ATTRIBUTE_LIST_SIZE);
	s_strncat(attributes,qt->Completeness,ATTRIBUTE_SIZE,ATTRIBUTE_LIST_SIZE);
	buf = writeString(attributes,DT_AttributeList,buf,len);
	buf = writeAny(qt->Term,DT_Term,buf,len);
	break;
      case TT_ResultSetID:
	buf = writeAny(qt->ResultSetID,DT_ResultSetID,buf,len);
	break;
      case TT_Operator:
	buf = writeString(qt->Operator,DT_Operator,buf,len);
	break;
      default:
	panic("Implementation error: Unknown term type %ld",
	      qt->TermType);
	break;
      }

  return(buf);
}

/*----------------------------------------------------------------------*/
#if !defined(IN_RMG) && !defined(PFS_THREADS)
/* Note uses strtok - not thread-safe */
static char* 
readQueryTerm(qt,buffer)
query_term** qt;
char* buffer;
{
  char* buf = buffer;
  char  *attributeList = NULL;
  char* operator = NULL;
  any* 	term;
  char* use = NULL;
  char* relation = NULL;
  char* position = NULL;
  char* structure = NULL;
  char* truncation = NULL;
  char* completeness;
  any*	resultSetID = NULL;
  data_tag tag;

  
  if (buffer == NULL) 
   { waislog(WLOG_ERROR,WLOG_HIGH,"null buffer in readQueryTerm");
     return(NULL);
   }

  tag = peekTag(buffer);

  switch(tag)
    { case DT_AttributeList:
	buf = readString(&attributeList,buf);
	buf = readAny(&term,buf);
	assert(P_IS_THIS_THREAD_MASTER()); /* strtok is thread unsafe */
	use = strtok(attributeList,AT_DELIMITER);
	relation = strtok(NULL,AT_DELIMITER);
	position = strtok(NULL,AT_DELIMITER);
	structure = strtok(NULL,AT_DELIMITER);
	truncation = strtok(NULL,AT_DELIMITER);
	completeness = strtok(NULL,AT_DELIMITER);
	*qt = makeAttributeTerm(use,relation,position,structure,
				truncation,completeness,term);
	s_free(attributeList);
	freeAny(term);
	break;
      case DT_ResultSetID:
	buf = readAny(&resultSetID,buf);
	*qt = makeResultSetTerm(resultSetID);	
	freeAny(resultSetID);
	break;
      case DT_Operator:
	buf = readString(&operator,buf);
	*qt = makeOperatorTerm(operator);
	s_free(operator);
	break;
      default:
	REPORT_READ_ERROR(buf);
	break;
      }
  
  return(buf);
}
#endif
/*----------------------------------------------------------------------*/

static unsigned long getQueryTermSize _AP((query_term* qt));

static unsigned long
getQueryTermSize(qt)
query_term* qt;

{
  unsigned long size;
  static char attributes[] = "11 22 33 44 55 66"; 

  switch (qt->TermType)
    { case TT_Attribute:
	size = writtenStringSize(DT_AttributeList,attributes);
	size += writtenAnySize(DT_Term,qt->Term);
	break;
      case TT_ResultSetID:
	size = writtenAnySize(DT_ResultSetID,qt->ResultSetID);
	break;
      case TT_Operator:
	size = writtenStringSize(DT_Operator,qt->Operator);
	break;
      default:
	panic("Implementation error: Unknown term type %ld",
	      qt->TermType);
	break;
      }

  return(size);
}

/*----------------------------------------------------------------------*/



any*
writeQuery(terms)
query_term** terms;
{
  any* info = NULL;
  char* writePos = NULL;
  char* data = NULL;
  unsigned long size = 0;
  long remaining = 0;
  long i;
  query_term* qt = NULL;

  if (terms == NULL)
    return(NULL);

  
  for (i = 0,qt = terms[i]; qt != NULL; qt = terms[++i])
    size += getQueryTermSize(qt);

  data = (char*)s_malloc((size_t)size);

  
  writePos = data;
  remaining = size;
  for (i = 0,qt = terms[i]; qt != NULL; qt = terms[++i])
    writePos = writeQueryTerm(qt,writePos,&remaining);

  info = makeAny(size,data);

  return(info);
}

/*----------------------------------------------------------------------*/
#if !defined(IN_RMG) && !defined(PFS_THREADS)
query_term**
readQuery(info)
any *info;
{
  char* readPos = info->bytes;
  query_term** terms = NULL;
  query_term* qt = NULL;
  long numTerms = 0;


  while (readPos != NULL &&
	 readPos < info->bytes + info->size)
    { readPos = readQueryTerm(&qt,readPos);
      if (terms == NULL)
	{ terms = (query_term**)s_malloc((size_t)(sizeof(query_term*)*2));
	}
      else
	{ terms = 
	    (query_term**)s_realloc((char*)terms,
				    (size_t)(sizeof(query_term*)*(numTerms+2)));
	  }
      terms[numTerms++] = qt;
      terms[numTerms] = NULL;
    }

  return(terms);
}
#endif /* !defined(IN_RMG) && !defined(PFS_THREADS) */

/*----------------------------------------------------------------------*/
