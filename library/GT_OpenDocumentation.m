function GT_OpenDocumentation(page)
    thisDirectory = extractBefore(mfilename('fullpath'),strlength(mfilename('fullpath')) - strlength(mfilename) + 1);
    htmlFile = fullfile(thisDirectory, '..', 'documentation', 'html', page);
    web(htmlFile);
end

