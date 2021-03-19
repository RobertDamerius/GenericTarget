function info = DecodeIndexFile(filename)
    %GT.DecodeIndexFile Decode index file created by the generic target application.
    % 
    % PARAMETERS
    % filename ... The name of the index file to be decoded.
    % 
    % RETURN
    % info ... Structure that contains information about the log.
    % 
    % REVISION HISTORY
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % Version     Author                 Changes
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    % 20210319    Robert Damerius        Initial release.
    % 
    % ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    % Check input and set default output
    assert(ischar(filename), 'GT.DecodeIndexFile(): Input "filename" must be a string!');
    info = struct();

    % Open index file
    fp = fopen(filename,'r');
    if(fp < 0)
        error('Could not open index file "%s"\n',filename);
    end
    fprintf('[DECODE] Decoding index file "%s" ... ',filename);

    % Header should be: 'G' (71), 'T' (84), 'I' (73), 'D' (68), 'X' (88)
    bytesHeader = uint8(fread(fp, 5));
    if((5 ~= length(bytesHeader)) || (uint8(71) ~= bytesHeader(1)) || (uint8(84) ~= bytesHeader(2)) || (uint8(73) ~= bytesHeader(3)) || (uint8(68) ~= bytesHeader(4)) || (uint8(88) ~= bytesHeader(5)))
        fclose(fp);
        error('Invalid header of file: "%s"\n',filename);
    end

    % Decode date
    bytesDate = uint8(fread(fp, 9));
    if(9 ~= length(bytesDate))
        fclose(fp);
        error('Invalid header of file: "%s"\n',filename);
    end
    info.dateUTC.year = uint32(bitor(bitor(bitshift(uint32(bytesDate(1)),24),bitshift(uint32(bytesDate(2)),16)),bitor(bitshift(uint32(bytesDate(3)),8),uint32(bytesDate(4)))));
    info.dateUTC.month = bytesDate(5);
    info.dateUTC.mday = bytesDate(6);
    info.dateUTC.hour = bytesDate(7);
    info.dateUTC.minute = bytesDate(8);
    info.dateUTC.second = bytesDate(9);

    % Number of IDs
    bytesNumIDs = uint8(fread(fp, 4));
    if(4 ~= length(bytesNumIDs))
        fclose(fp);
        error('Invalid header of file: "%s"\n',filename);
    end
    numIDs = uint32(bitor(bitor(bitshift(uint32(bytesNumIDs(1)),24),bitshift(uint32(bytesNumIDs(2)),16)),bitor(bitshift(uint32(bytesNumIDs(3)),8),uint32(bytesNumIDs(4)))));

    % All IDs (4 byte per ID)
    info.listOfIDs = uint32(zeros(numIDs,1));
    for i = uint32(1):numIDs
        bytes = uint8(fread(fp, 4));
        if(4 ~= length(bytes))
            fclose(fp);
            error('Invalid header of file: "%s"\n',filename);
        end
        info.listOfIDs(i) = uint32(bitor(bitor(bitshift(uint32(bytes(1)),24),bitshift(uint32(bytes(2)),16)),bitor(bitshift(uint32(bytes(3)),8),uint32(bytes(4)))));
    end

    % Version (zero-terminated string)
    info.version = char.empty();
    while(true)
        byte = uint8(fread(fp, 1));
        if(1 ~= length(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!\n',length(byte),filename);
        end
        if(~byte)
            break;
        end
        info.version = [info.version, char(byte)];
    end

    % Model name (zero-terminated string)
    info.modelName = char.empty();
    while(true)
        byte = uint8(fread(fp, 1));
        if(1 ~= length(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!\n',length(byte),filename);
        end
        if(~byte)
            break;
        end
        info.modelName = [info.modelName, char(byte)];
    end

    % dateOfCompilation (zero-terminated string)
    info.dateOfCompilation = char.empty();
    while(true)
        byte = uint8(fread(fp, 1));
        if(1 ~= length(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!\n',length(byte),filename);
        end
        if(~byte)
            break;
        end
        info.dateOfCompilation = [info.dateOfCompilation, char(byte)];
    end
    fclose(fp);
    fprintf('OK\n\n');
end

