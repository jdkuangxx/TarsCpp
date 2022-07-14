﻿/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#include "code_generator.h"

using namespace std;

string CodeGenerator::printHeaderRemark(const string &sTypeName)
{
    ostringstream s;
    s << "// **********************************************************************" << endl;
    s << "// Parsed By " << IDL_NAMESPACE_STR << "Parser(" << PARSER_VERSION << "), Generated By " << EXECUTE_FILENAME << "(" << GENERATOR_VERSION << ")" << endl;
    s << "// " << IDL_NAMESPACE_STR << "Parser Maintained By <" << TC_Common::upper(IDL_NAMESPACE_STR) << "> and " << EXECUTE_FILENAME << " Maintained By <superzheng>" << endl;
    s << "// Generated from \"" << TC_File::extractFileName(_sIdlFile) << "\" by " <<
            (_bEntry ? sTypeName : (_bMinimalMembers ? "Minimal" : "Relation")) << " Mode" << endl;
    s << "// **********************************************************************" << endl;
    s << endl;

    return s.str();
}

void CodeGenerator::createFile(const string &file, const bool bEntry)
{
    _sIdlFile = getRealFileInfo(file);
    _bEntry = bEntry;

    g_parse->parse(_sIdlFile);

    vector<ContextPtr> contexts = g_parse->getContexts();

    for(size_t i = 0; i < contexts.size(); i++)
    {
        if (_sIdlFile == contexts[i]->getFileName())
        {
            scan(_sIdlFile, true);                                  // collect idl symbols

            if (!_bClient && !_bServer)
            {
                if (_bTS)
                {
                    generateTS(contexts[i]);                        // generate .ts
                }
                else
                {
                    generateJS(contexts[i]);                        // generate .js
                    if (_bDTS) generateDTS(contexts[i]);            // generate .d.ts
                }
            }

            if (_bClient)
            {
                if (_bTS)
                {
                    if (!generateTSProxy(contexts[i])) return;      // generate .ts for proxy classes
                }
                else
                {
                    if (!generateJSProxy(contexts[i])) return;      // generate .js for proxy classes
                    if (_bDTS) generateDTSProxy(contexts[i]);       // generate .d.ts for proxy classes
                }
            }

            if (_bServer)
            {
                if (_bTS)
                {
                    if (!generateTSServer(contexts[i])) return;      // generate .ts for server classes
                    generateTSServerImp(contexts[i]);                // generate .ts for server implementations
                }
                else
                {
                    if (!generateJSServer(contexts[i])) return;      // generate .js for server classes
                    if (_bDTS) generateDTSServer(contexts[i]);       // generate .d.ts for server classes
                    generateJSServerImp(contexts[i]);                // generate .js for server implementations
                }
            }

            vector<string> files = contexts[i]->getIncludes();
            for (size_t ii = 0; _bRecursive && ii < files.size(); ii++)
            {
                CodeGenerator node;
                node.setRpcPath(_sRpcPath);
                node.setStreamPath(_sStreamPath);
                node.setTargetPath(_sToPath);
                node.setRecursive(_bRecursive);
                node.setUseSpecialPath(_bUseSpecialPath);
                node.setLongType(_iLongType);
                node.setStringBinaryEncoding(_bStringBinaryEncoding);
                node.setMinimalMembers(_bMinimalMembers);
                node.setDependent(_depMembers);
                node.setEnableTS(_bTS);
                node.setEnableDTS(_bDTS);

                node.createFile(files[ii], false);
            }
        }
    }
}