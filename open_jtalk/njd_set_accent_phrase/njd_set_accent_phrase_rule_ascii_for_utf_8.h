/* ----------------------------------------------------------------- */
/*           The Japanese TTS System "Open JTalk"                    */
/*           developed by HTS Working Group                          */
/*           http://open-jtalk.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2008-2016  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef NJD_SET_ACCENT_PHRASE_RULE_H
#define NJD_SET_ACCENT_PHRASE_RULE_H

#ifdef __cplusplus
#define NJD_SET_ACCENT_PHRASE_RULE_H_START extern "C" {
#define NJD_SET_ACCENT_PHRASE_RULE_H_END   }
#else
#define NJD_SET_ACCENT_PHRASE_RULE_H_START
#define NJD_SET_ACCENT_PHRASE_RULE_H_END
#endif                          /* __CPLUSPLUS */

NJD_SET_ACCENT_PHRASE_RULE_H_START;

/*
  Rule 01 \xe3\x83\x87\xe3\x83\x95\xe3\x82\xa9\xe3\x83\xab\xe3\x83\x88\xe3\x81\xaf\xe3\x81\x8f\xe3\x81\xa3\xe3\x81\xa4\xe3\x81\x91\xe3\x82\x8b
  Rule 02 \xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xae\xe9\x80\xa3\xe7\xb6\x9a\xe3\x81\xaf\xe3\x81\x8f\xe3\x81\xa3\xe3\x81\xa4\xe3\x81\x91\xe3\x82\x8b
  Rule 03 \xe3\x80\x8c\xe5\xbd\xa2\xe5\xae\xb9\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xae\xe5\xbe\x8c\xe3\x81\xab\xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\x8c\xe3\x81\x8d\xe3\x81\x9f\xe3\x82\x89\xe5\x88\xa5\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 04 \xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e,\xe5\xbd\xa2\xe5\xae\xb9\xe5\x8b\x95\xe8\xa9\x9e\xe8\xaa\x9e\xe5\xb9\xb9\xe3\x80\x8d\xe3\x81\xae\xe5\xbe\x8c\xe3\x81\xab\xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\x8c\xe3\x81\x8d\xe3\x81\x9f\xe3\x82\x89\xe5\x88\xa5\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 05 \xe3\x80\x8c\xe5\x8b\x95\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xae\xe5\xbe\x8c\xe3\x81\xab\xe3\x80\x8c\xe5\xbd\xa2\xe5\xae\xb9\xe8\xa9\x9e\xe3\x80\x8dor\xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\x8c\xe3\x81\x8d\xe3\x81\x9f\xe3\x82\x89\xe5\x88\xa5\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 06 \xe3\x80\x8c\xe5\x89\xaf\xe8\xa9\x9e\xe3\x80\x8d\xef\xbc\x8c\xe3\x80\x8c\xe6\x8e\xa5\xe7\xb6\x9a\xe8\xa9\x9e\xe3\x80\x8d\xef\xbc\x8c\xe3\x80\x8c\xe9\x80\xa3\xe4\xbd\x93\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xaf\xe5\x8d\x98\xe7\x8b\xac\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 07 \xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e,\xe5\x89\xaf\xe8\xa9\x9e\xe5\x8f\xaf\xe8\x83\xbd\xe3\x80\x8d\xef\xbc\x88\xe3\x81\x99\xe3\x81\xb9\xe3\x81\xa6\xef\xbc\x8c\xe3\x81\xaa\xe3\x81\xa9\xef\xbc\x89\xe3\x81\xaf\xe5\x8d\x98\xe7\x8b\xac\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 08 \xe3\x80\x8c\xe5\x8a\xa9\xe8\xa9\x9e\xe3\x80\x8dor\xe3\x80\x8c\xe5\x8a\xa9\xe5\x8b\x95\xe8\xa9\x9e\xe3\x80\x8d\xef\xbc\x88\xe4\xbb\x98\xe5\xb1\x9e\xe8\xaa\x9e\xef\xbc\x89\xe3\x81\xaf\xe5\x89\x8d\xe3\x81\xab\xe3\x81\x8f\xe3\x81\xa3\xe3\x81\xa4\xe3\x81\x91\xe3\x82\x8b
  Rule 09 \xe3\x80\x8c\xe5\x8a\xa9\xe8\xa9\x9e\xe3\x80\x8dor\xe3\x80\x8c\xe5\x8a\xa9\xe5\x8b\x95\xe8\xa9\x9e\xe3\x80\x8d\xef\xbc\x88\xe4\xbb\x98\xe5\xb1\x9e\xe8\xaa\x9e\xef\xbc\x89\xe3\x81\xae\xe5\xbe\x8c\xe3\x81\xae\xe3\x80\x8c\xe5\x8a\xa9\xe8\xa9\x9e\xe3\x80\x8d\xef\xbc\x8c\xe3\x80\x8c\xe5\x8a\xa9\xe5\x8b\x95\xe8\xa9\x9e\xe3\x80\x8d\xe4\xbb\xa5\xe5\xa4\x96\xef\xbc\x88\xe8\x87\xaa\xe7\xab\x8b\xe8\xaa\x9e\xef\xbc\x89\xe3\x81\xaf\xe5\x88\xa5\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 10 \xe3\x80\x8c*,\xe6\x8e\xa5\xe5\xb0\xbe\xe3\x80\x8d\xe3\x81\xae\xe5\xbe\x8c\xe3\x81\xae\xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xaf\xe5\x88\xa5\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 11 \xe3\x80\x8c\xe5\xbd\xa2\xe5\xae\xb9\xe8\xa9\x9e,\xe9\x9d\x9e\xe8\x87\xaa\xe7\xab\x8b\xe3\x80\x8d\xe3\x81\xaf\xe3\x80\x8c\xe5\x8b\x95\xe8\xa9\x9e,\xe9\x80\xa3\xe7\x94\xa8*\xe3\x80\x8dor\xe3\x80\x8c\xe5\xbd\xa2\xe5\xae\xb9\xe8\xa9\x9e,\xe9\x80\xa3\xe7\x94\xa8*\xe3\x80\x8dor\xe3\x80\x8c\xe5\x8a\xa9\xe8\xa9\x9e,\xe6\x8e\xa5\xe7\xb6\x9a\xe5\x8a\xa9\xe8\xa9\x9e,\xe3\x81\xa6\xe3\x80\x8dor\xe3\x80\x8c\xe5\x8a\xa9\xe8\xa9\x9e,\xe6\x8e\xa5\xe7\xb6\x9a\xe5\x8a\xa9\xe8\xa9\x9e,\xe3\x81\xa7\xe3\x80\x8d\xe3\x81\xab\xe6\x8e\xa5\xe7\xb6\x9a\xe3\x81\x99\xe3\x82\x8b\xe5\xa0\xb4\xe5\x90\x88\xe3\x81\xab\xe5\x89\x8d\xe3\x81\xab\xe3\x81\x8f\xe3\x81\xa3\xe3\x81\xa4\xe3\x81\x91\xe3\x82\x8b
  Rule 12 \xe3\x80\x8c\xe5\x8b\x95\xe8\xa9\x9e,\xe9\x9d\x9e\xe8\x87\xaa\xe7\xab\x8b\xe3\x80\x8d\xe3\x81\xaf\xe3\x80\x8c\xe5\x8b\x95\xe8\xa9\x9e,\xe9\x80\xa3\xe7\x94\xa8*\xe3\x80\x8dor\xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e,\xe3\x82\xb5\xe5\xa4\x89\xe6\x8e\xa5\xe7\xb6\x9a\xe3\x80\x8d\xe3\x81\xab\xe6\x8e\xa5\xe7\xb6\x9a\xe3\x81\x99\xe3\x82\x8b\xe5\xa0\xb4\xe5\x90\x88\xe3\x81\xab\xe5\x89\x8d\xe3\x81\xab\xe3\x81\x8f\xe3\x81\xa3\xe3\x81\xa4\xe3\x81\x91\xe3\x82\x8b
  Rule 13 \xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xae\xe5\xbe\x8c\xe3\x81\xab\xe3\x80\x8c\xe5\x8b\x95\xe8\xa9\x9e\xe3\x80\x8dor\xe3\x80\x8c\xe5\xbd\xa2\xe5\xae\xb9\xe8\xa9\x9e\xe3\x80\x8dor\xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e,\xe5\xbd\xa2\xe5\xae\xb9\xe5\x8b\x95\xe8\xa9\x9e\xe8\xaa\x9e\xe5\xb9\xb9\xe3\x80\x8d\xe3\x81\x8c\xe3\x81\x8d\xe3\x81\x9f\xe3\x82\x89\xe5\x88\xa5\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 14 \xe3\x80\x8c\xe8\xa8\x98\xe5\x8f\xb7\xe3\x80\x8d\xe3\x81\xaf\xe5\x8d\x98\xe7\x8b\xac\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 15 \xe3\x80\x8c\xe6\x8e\xa5\xe9\xa0\xad\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xaf\xe5\x8d\x98\xe7\x8b\xac\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 16 \xe3\x80\x8c*,*,*,\xe5\xa7\x93\xe3\x80\x8d\xe3\x81\xae\xe5\xbe\x8c\xe3\x81\xae\xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xaf\xe5\x88\xa5\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 17 \xe3\x80\x8c\xe5\x90\x8d\xe8\xa9\x9e\xe3\x80\x8d\xe3\x81\xae\xe5\xbe\x8c\xe3\x81\xae\xe3\x80\x8c*,*,*,\xe5\x90\x8d\xe3\x80\x8d\xe3\x81\xaf\xe5\x88\xa5\xe3\x81\xae\xe3\x82\xa2\xe3\x82\xaf\xe3\x82\xbb\xe3\x83\xb3\xe3\x83\x88\xe5\x8f\xa5\xe3\x81\xab
  Rule 18 \xe3\x80\x8c*,\xe6\x8e\xa5\xe5\xb0\xbe\xe3\x80\x8d\xe3\x81\xaf\xe5\x89\x8d\xe3\x81\xab\xe3\x81\x8f\xe3\x81\xa3\xe3\x81\xa4\xe3\x81\x91\xe3\x82\x8b
*/

#define NJD_SET_ACCENT_PHRASE_MEISHI "\xe5\x90\x8d\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_KEIYOUSHI "\xe5\xbd\xa2\xe5\xae\xb9\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_DOUSHI "\xe5\x8b\x95\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_FUKUSHI "\xe5\x89\xaf\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_SETSUZOKUSHI "\xe6\x8e\xa5\xe7\xb6\x9a\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_RENTAISHI "\xe9\x80\xa3\xe4\xbd\x93\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_JODOUSHI "\xe5\x8a\xa9\xe5\x8b\x95\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_JOSHI "\xe5\x8a\xa9\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_KIGOU "\xe8\xa8\x98\xe5\x8f\xb7"
#define NJD_SET_ACCENT_PHRASE_KEIYOUDOUSHI_GOKAN "\xe5\xbd\xa2\xe5\xae\xb9\xe5\x8b\x95\xe8\xa9\x9e\xe8\xaa\x9e\xe5\xb9\xb9"
#define NJD_SET_ACCENT_PHRASE_FUKUSHI_KANOU "\xe5\x89\xaf\xe8\xa9\x9e\xe5\x8f\xaf\xe8\x83\xbd"
#define NJD_SET_ACCENT_PHRASE_SETSUBI "\xe6\x8e\xa5\xe5\xb0\xbe"
#define NJD_SET_ACCENT_PHRASE_HIJIRITSU "\xe9\x9d\x9e\xe8\x87\xaa\xe7\xab\x8b"
#define NJD_SET_ACCENT_PHRASE_RENYOU "\xe9\x80\xa3\xe7\x94\xa8"
#define NJD_SET_ACCENT_PHRASE_SETSUZOKUJOSHI "\xe6\x8e\xa5\xe7\xb6\x9a\xe5\x8a\xa9\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_SAHEN_SETSUZOKU "\xe3\x82\xb5\xe5\xa4\x89\xe6\x8e\xa5\xe7\xb6\x9a"
#define NJD_SET_ACCENT_PHRASE_TE "\xe3\x81\xa6"
#define NJD_SET_ACCENT_PHRASE_DE "\xe3\x81\xa7"
#define NJD_SET_ACCENT_PHRASE_SETTOUSHI "\xe6\x8e\xa5\xe9\xa0\xad\xe8\xa9\x9e"
#define NJD_SET_ACCENT_PHRASE_SEI "\xe5\xa7\x93"
#define NJD_SET_ACCENT_PHRASE_MEI "\xe5\x90\x8d"

NJD_SET_ACCENT_PHRASE_RULE_H_END;

#endif                          /* !NJD_SET_ACCENT_PHRASE_RULE_H */
