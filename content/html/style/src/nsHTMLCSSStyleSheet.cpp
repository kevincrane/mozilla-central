/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is 
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsIHTMLCSSStyleSheet.h"
#include "nsIArena.h"
#include "nsCRT.h"
#include "nsIAtom.h"
#include "nsIURL.h"
#include "nsISupportsArray.h"
#include "nsCSSPseudoElements.h"
#include "nsIHTMLContent.h"
#include "nsIStyleRule.h"
#include "nsIFrame.h"
#include "nsICSSStyleRule.h"
#include "nsIStyleRuleProcessor.h"
#include "nsIPresContext.h"
#include "nsIDocument.h"
#include "nsCOMPtr.h"

#include "nsIStyleSet.h"
#include "nsRuleWalker.h"

class CSSFirstLineRule : public nsIStyleRule {
public:
  CSSFirstLineRule(nsIHTMLCSSStyleSheet* aSheet);
  virtual ~CSSFirstLineRule();

  NS_DECL_ISUPPORTS

  NS_IMETHOD GetStyleSheet(nsIStyleSheet*& aSheet) const;
  
  // The new mapping function.
  NS_IMETHOD MapRuleInfoInto(nsRuleData* aRuleData);

#ifdef DEBUG
  NS_IMETHOD List(FILE* out = stdout, PRInt32 aIndent = 0) const;
#endif

  nsIHTMLCSSStyleSheet*  mSheet;
};

CSSFirstLineRule::CSSFirstLineRule(nsIHTMLCSSStyleSheet* aSheet)
  : mSheet(aSheet)
{
}

CSSFirstLineRule::~CSSFirstLineRule()
{
}

NS_IMPL_ISUPPORTS1(CSSFirstLineRule, nsIStyleRule)

NS_IMETHODIMP
CSSFirstLineRule::GetStyleSheet(nsIStyleSheet*& aSheet) const
{
  NS_IF_ADDREF(mSheet);
  aSheet = mSheet;
  return NS_OK;
}

NS_IMETHODIMP
CSSFirstLineRule::MapRuleInfoInto(nsRuleData* aData)
{
  if (!aData)
    return NS_OK;

  if (aData->mSID == eStyleStruct_Border && aData->mMarginData) {
    // Disable the border.
    nsCSSValue styleVal(NS_STYLE_BORDER_STYLE_NONE, eCSSUnit_Enumerated);
    if (aData->mMarginData->mBorderStyle->mLeft.GetUnit() == eCSSUnit_Null)
      aData->mMarginData->mBorderStyle->mLeft = styleVal;
    if (aData->mMarginData->mBorderStyle->mRight.GetUnit() == eCSSUnit_Null)
      aData->mMarginData->mBorderStyle->mRight = styleVal;
    if (aData->mMarginData->mBorderStyle->mTop.GetUnit() == eCSSUnit_Null)
      aData->mMarginData->mBorderStyle->mTop = styleVal;
    if (aData->mMarginData->mBorderStyle->mBottom.GetUnit() == eCSSUnit_Null)
      aData->mMarginData->mBorderStyle->mBottom = styleVal;
  }

  return NS_OK;
}

#ifdef DEBUG
NS_IMETHODIMP
CSSFirstLineRule::List(FILE* out, PRInt32 aIndent) const
{
  return NS_OK;
}
#endif

// -----------------------------------------------------------

class CSSFirstLetterRule : public CSSFirstLineRule {
public:
  CSSFirstLetterRule(nsIHTMLCSSStyleSheet* aSheet);
};

CSSFirstLetterRule::CSSFirstLetterRule(nsIHTMLCSSStyleSheet* aSheet)
  : CSSFirstLineRule(aSheet)
{
}

// -----------------------------------------------------------

class HTMLCSSStyleSheetImpl : public nsIHTMLCSSStyleSheet,
                              public nsIStyleRuleProcessor {
public:
  HTMLCSSStyleSheetImpl();

  NS_DECL_ISUPPORTS

  // basic style sheet data
  NS_IMETHOD Init(nsIURI* aURL, nsIDocument* aDocument);
  NS_IMETHOD Reset(nsIURI* aURL);
  NS_IMETHOD GetURL(nsIURI*& aURL) const;
  NS_IMETHOD GetTitle(nsString& aTitle) const;
  NS_IMETHOD GetType(nsString& aType) const;
  NS_IMETHOD GetMediumCount(PRInt32& aCount) const;
  NS_IMETHOD GetMediumAt(PRInt32 aIndex, nsIAtom*& aMedium) const;
  NS_IMETHOD_(PRBool) UseForMedium(nsIAtom* aMedium) const;

  NS_IMETHOD GetApplicable(PRBool& aApplicable) const;
  
  NS_IMETHOD SetEnabled(PRBool aEnabled);

  NS_IMETHOD GetComplete(PRBool& aComplete) const;
  NS_IMETHOD SetComplete();

  // style sheet owner info
  NS_IMETHOD GetParentSheet(nsIStyleSheet*& aParent) const;  // will be null
  NS_IMETHOD GetOwningDocument(nsIDocument*& aDocument) const;
  NS_IMETHOD SetOwningDocument(nsIDocument* aDocument);

  NS_IMETHOD GetStyleRuleProcessor(nsIStyleRuleProcessor*& aProcessor,
                                   nsIStyleRuleProcessor* aPrevProcessor);

  // nsIStyleRuleProcessor api
  NS_IMETHOD RulesMatching(ElementRuleProcessorData* aData,
                           nsIAtom* aMedium);

  NS_IMETHOD RulesMatching(PseudoRuleProcessorData* aData,
                           nsIAtom* aMedium);

  NS_IMETHOD HasStateDependentStyle(StateRuleProcessorData* aData,
                                    nsIAtom* aMedium,
                                    PRBool* aResult);

  NS_IMETHOD HasAttributeDependentStyle(AttributeRuleProcessorData* aData,
                                        nsIAtom* aMedium,
                                        PRBool* aResult);

#ifdef DEBUG
  virtual void List(FILE* out = stdout, PRInt32 aIndent = 0) const;
#endif

private: 
  // These are not supported and are not implemented! 
  HTMLCSSStyleSheetImpl(const HTMLCSSStyleSheetImpl& aCopy); 
  HTMLCSSStyleSheetImpl& operator=(const HTMLCSSStyleSheetImpl& aCopy); 

protected:
  virtual ~HTMLCSSStyleSheetImpl();

protected:
  nsIURI*         mURL;
  nsIDocument*    mDocument;

  CSSFirstLineRule* mFirstLineRule;
  CSSFirstLetterRule* mFirstLetterRule;
};


HTMLCSSStyleSheetImpl::HTMLCSSStyleSheetImpl()
  : nsIHTMLCSSStyleSheet(),
    mRefCnt(0),
    mURL(nsnull),
    mDocument(nsnull),
    mFirstLineRule(nsnull),
    mFirstLetterRule(nsnull)
{
}

HTMLCSSStyleSheetImpl::~HTMLCSSStyleSheetImpl()
{
  NS_RELEASE(mURL);
  if (nsnull != mFirstLineRule) {
    mFirstLineRule->mSheet = nsnull;
    NS_RELEASE(mFirstLineRule);
  }
  if (nsnull != mFirstLetterRule) {
    mFirstLetterRule->mSheet = nsnull;
    NS_RELEASE(mFirstLetterRule);
  }
}

NS_IMPL_ISUPPORTS3(HTMLCSSStyleSheetImpl,
                   nsIHTMLCSSStyleSheet,
                   nsIStyleSheet,
                   nsIStyleRuleProcessor)

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetStyleRuleProcessor(nsIStyleRuleProcessor*& aProcessor,
                                             nsIStyleRuleProcessor* /*aPrevProcessor*/)
{
  aProcessor = this;
  NS_ADDREF(aProcessor);
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::RulesMatching(ElementRuleProcessorData* aData,
                                     nsIAtom* aMedium)
{
  nsIStyledContent *styledContent = aData->mStyledContent;
  
  if (styledContent) {
    // just get the one and only style rule from the content's STYLE attribute
    nsCOMPtr<nsIStyleRule> rule;
    styledContent->GetInlineStyleRule(getter_AddRefs(rule));
    if (rule)
      aData->mRuleWalker->Forward(rule, PR_TRUE);
  }

  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::RulesMatching(PseudoRuleProcessorData* aData,
                                     nsIAtom* aMedium)
{
  // We only want to add these rules if there are real :first-letter or
  // :first-line rules that cause a pseudo-element frame to be created.
  // Otherwise the use of ProbePseudoStyleContextFor will prevent frame
  // creation, and adding rules here would cause it.
  if (aData->mRuleWalker->AtRoot())
    return NS_OK;

  nsIAtom* pseudoTag = aData->mPseudoTag;
  if (pseudoTag == nsCSSPseudoElements::firstLine) {
    if (!mFirstLineRule) {
      mFirstLineRule = new CSSFirstLineRule(this);
      if (!mFirstLineRule)
        return NS_ERROR_OUT_OF_MEMORY;
      NS_ADDREF(mFirstLineRule);
    }
    aData->mRuleWalker->Forward(mFirstLineRule);
  }
  else if (pseudoTag == nsCSSPseudoElements::firstLetter) {
    if (!mFirstLetterRule) {
      mFirstLetterRule = new CSSFirstLetterRule(this);
      if (!mFirstLetterRule)
        return NS_ERROR_OUT_OF_MEMORY;
      NS_ADDREF(mFirstLetterRule);
    }
    aData->mRuleWalker->Forward(mFirstLetterRule);
  } 
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::Init(nsIURI* aURL, nsIDocument* aDocument)
{
  NS_PRECONDITION(aURL && aDocument, "null ptr");
  if (! aURL || ! aDocument)
    return NS_ERROR_NULL_POINTER;

  if (mURL || mDocument)
    return NS_ERROR_ALREADY_INITIALIZED;

  mDocument = aDocument; // not refcounted!
  mURL = aURL;
  NS_ADDREF(mURL);
  return NS_OK;
}

// Test if style is dependent on content state
NS_IMETHODIMP
HTMLCSSStyleSheetImpl::HasStateDependentStyle(StateRuleProcessorData* aData,
                                              nsIAtom* aMedium,
                                              PRBool* aResult)
{
  *aResult = PR_FALSE;
  return NS_OK;
}

// Test if style is dependent on attribute
NS_IMETHODIMP
HTMLCSSStyleSheetImpl::HasAttributeDependentStyle(AttributeRuleProcessorData* aData,
                                                  nsIAtom* aMedium,
                                                  PRBool* aResult)
{
  *aResult = PR_FALSE;
  return NS_OK;
}



NS_IMETHODIMP 
HTMLCSSStyleSheetImpl::Reset(nsIURI* aURL)
{
  NS_IF_RELEASE(mURL);
  mURL = aURL;
  NS_ADDREF(mURL);
  if (nsnull != mFirstLineRule) {
    mFirstLineRule->mSheet = nsnull;
    NS_RELEASE(mFirstLineRule);
  }
  if (nsnull != mFirstLetterRule) {
    mFirstLetterRule->mSheet = nsnull;
    NS_RELEASE(mFirstLetterRule);
  }
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetURL(nsIURI*& aURL) const
{
  NS_IF_ADDREF(mURL);
  aURL = mURL;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetTitle(nsString& aTitle) const
{
  aTitle.Assign(NS_LITERAL_STRING("Internal HTML/CSS Style Sheet"));
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetType(nsString& aType) const
{
  aType.Assign(NS_LITERAL_STRING("text/html"));
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetMediumCount(PRInt32& aCount) const
{
  aCount = 0;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetMediumAt(PRInt32 aIndex, nsIAtom*& aMedium) const
{
  aMedium = nsnull;
  return NS_ERROR_INVALID_ARG;
}

NS_IMETHODIMP_(PRBool)
HTMLCSSStyleSheetImpl::UseForMedium(nsIAtom* aMedium) const
{
  return PR_TRUE; // works for all media
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetApplicable(PRBool& aApplicable) const
{
  aApplicable = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::SetEnabled(PRBool aEnabled)
{ // these can't be disabled
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetComplete(PRBool& aComplete) const
{
  aComplete = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::SetComplete()
{
  return NS_OK;
}

// style sheet owner info
NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetParentSheet(nsIStyleSheet*& aParent) const
{
  aParent = nsnull;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetOwningDocument(nsIDocument*& aDocument) const
{
  NS_IF_ADDREF(mDocument);
  aDocument = mDocument;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::SetOwningDocument(nsIDocument* aDocument)
{
  mDocument = aDocument;
  return NS_OK;
}

#ifdef DEBUG
void HTMLCSSStyleSheetImpl::List(FILE* out, PRInt32 aIndent) const
{
  // Indent
  for (PRInt32 index = aIndent; --index >= 0; ) fputs("  ", out);

  fputs("HTML CSS Style Sheet: ", out);
  nsCAutoString urlSpec;
  mURL->GetSpec(urlSpec);
  if (!urlSpec.IsEmpty()) {
    fputs(urlSpec.get(), out);
  }
  fputs("\n", out);
}
#endif

// XXX For backwards compatibility and convenience
NS_EXPORT nsresult
  NS_NewHTMLCSSStyleSheet(nsIHTMLCSSStyleSheet** aInstancePtrResult, nsIURI* aURL,
                          nsIDocument* aDocument)
{
  nsresult rv;
  nsIHTMLCSSStyleSheet* sheet;
  if (NS_FAILED(rv = NS_NewHTMLCSSStyleSheet(&sheet)))
    return rv;

  if (NS_FAILED(rv = sheet->Init(aURL, aDocument))) {
    NS_RELEASE(sheet);
    return rv;
  }

  *aInstancePtrResult = sheet;
  return NS_OK;
}

NS_EXPORT nsresult
  NS_NewHTMLCSSStyleSheet(nsIHTMLCSSStyleSheet** aInstancePtrResult)
{
  if (aInstancePtrResult == nsnull) {
    return NS_ERROR_NULL_POINTER;
  }

  HTMLCSSStyleSheetImpl*  it = new HTMLCSSStyleSheetImpl();

  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  NS_ADDREF(it);
  *aInstancePtrResult = it;
  return NS_OK;
}
