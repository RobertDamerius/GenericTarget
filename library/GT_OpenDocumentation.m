function GT_OpenDocumentation(page)
    if nargin < 1
        page = 'index.html';
    end
    thisDirectory = extractBefore(mfilename('fullpath'),strlength(mfilename('fullpath')) - strlength(mfilename) + 1);
    htmlFile = fullfile(thisDirectory, '..', 'documentation', 'html', page);
    web(htmlFile);
end

