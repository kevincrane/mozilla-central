/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

/*

  Private interface to the XBL Binding

*/

#ifndef nsIBINDABLE_CONTENT_h__
#define nsIBINDABLE_CONTENT_h__

#include "nsString.h"
#include "nsISupports.h"

class nsIContent;
class nsIXBLBinding;

// {55D70FE0-C8E5-11d3-97FB-00400553EEF0}
#define NS_IBINDABLE_CONTENT_IID \
{ 0x55d70fe0, 0xc8e5, 0x11d3, { 0x97, 0xfb, 0x0, 0x40, 0x5, 0x53, 0xee, 0xf0 } }

class nsIBindableContent : public nsISupports
{
public:
  static const nsIID& GetIID() { static nsIID iid = NS_IBINDABLE_CONTENT_IID; return iid; }

  NS_IMETHOD SetBinding(nsIXBLBinding* aBinding) = 0;
  NS_IMETHOD GetBinding(nsIXBLBinding** aResult) = 0;
};

#endif // nsIBINDABLE_CONTENT_h__
