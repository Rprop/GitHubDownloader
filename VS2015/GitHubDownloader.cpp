/********************************************************************
	Created:	2016/07/20  10:28
	Filename: 	GitHubDownloader.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>
#include <stdlib.h>

//-------------------------------------------------------------------------

const String root_dir   = _R("zlib-intel/"); // local directory
const String github_dir = _R("https://github.com/dotnet/corefx/tree/master/src/Native/Windows/clrcompression/zlib-intel/");
const String raw_dir    = _R("https://raw.githubusercontent.com/dotnet/corefx/master/src/Native/Windows/clrcompression/zlib-intel/");

//-------------------------------------------------------------------------

int main()
{
	printf("creating directory %s...", RT2A(root_dir).toGBK());
	Directory::Exist(root_dir) || Directory::Create(root_dir);
	printf(" done\n");

	printf("fetching %s...", RT2A(github_dir).toGBK());
	String github_page = WebClient::GetResponseText(github_dir);
	printf(" %s\n", github_page.IsNullOrEmpty() ? "failed" : "succeed");

	ManagedObject<StringArray> github_contents = github_page.MatchAll(_T("<td class=\"content\">"), _T("</td>"));
	if (github_contents && github_contents->Length > 0) {
		printf("downloading %Id files from %s...\n", github_contents->Length, RT2A(raw_dir).toGBK());

		ManagedObject<ThreadPool> threads = new ThreadPool;

		foreachp(lpcontent, github_contents)
		{
			threads->InvokeLater<String *>([](String *lpfilename)
			{
				intptr_t retry_times = 3;
				do 
				{
					printf("->%s %Id\n", RT2A(*lpfilename).toGBK(), retry_times);
					if (WebClient::DownloadFile(raw_dir + *lpfilename, root_dir + *lpfilename)) {
						break;
					} //if

				} while (--retry_times > 0);

				printf("%s%s\n", retry_times > 0 ? "++" : "--",  RT2A(*lpfilename).toGBK());

				delete lpfilename;
			}, new String(lpcontent->Match(_T("title=\""), _T("\""))));
		}

		threads->Dispatch();
		threads->WaitForTasksComplete();
	} //if

	printf("finished.\n");

	return STATUS_SUCCESS;
}