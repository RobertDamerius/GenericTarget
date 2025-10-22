function OpenDocumentation(page)
    %GT.OpenDocumentation Open the HTML-based documentation of the Generic Target toolbox.
    % 
    % PARAMETER
    % page ... (optional) The page to be opened. The default value is 'index.html'.

    if nargin < 1
        page = 'index.html';
    end
    thisDirectory = extractBefore(mfilename('fullpath'), strlength(mfilename('fullpath')) - strlength(mfilename) + 1);
    htmlFile = fullfile(thisDirectory, '..', '..', 'documentation', 'html', page);
    web(htmlFile);
end

