function info = DecodeIndexFile(filename)
    %GT.DecodeIndexFile Decode index file created by the Generic Target application.
    % 
    % PARAMETERS
    % filename ... The name of the index file to be decoded.
    % 
    % RETURN
    % info ... Structure that contains information about the data recording.

    % Check input and set default output
    assert(ischar(filename), 'GT.DecodeIndexFile(): Input "filename" must be a string!');
    info = struct();

    % Open index file
    [fp,errmsg] = fopen(filename,'r');
    if(fp < 0)
        error('Could not open index file "%s": %s',filename,errmsg);
    end
    fprintf('[GENERIC TARGET] Decoding index file "%s" ...',filename);

    % Header should be: 'G' (71), 'T' (84), 'I' (73), 'D' (68), 'X' (88)
    bytesHeader = uint8(fread(fp, 5));
    if((5 ~= numel(bytesHeader)) || (uint8(71) ~= bytesHeader(1)) || (uint8(84) ~= bytesHeader(2)) || (uint8(73) ~= bytesHeader(3)) || (uint8(68) ~= bytesHeader(4)) || (uint8(88) ~= bytesHeader(5)))
        fclose(fp);
        error('Invalid header of file: "%s"',filename);
    end

    % Decode date
    bytesDate = uint8(fread(fp, 11));
    if(11 ~= numel(bytesDate))
        fclose(fp);
        error('Invalid header of file: "%s"',filename);
    end
    info.dateUTC.year = uint32(bitor(bitor(bitshift(uint32(bytesDate(1)),24),bitshift(uint32(bytesDate(2)),16)),bitor(bitshift(uint32(bytesDate(3)),8),uint32(bytesDate(4)))));
    info.dateUTC.month = bytesDate(5);
    info.dateUTC.mday = bytesDate(6);
    info.dateUTC.hour = bytesDate(7);
    info.dateUTC.minute = bytesDate(8);
    info.dateUTC.second = bytesDate(9);
    info.dateUTC.millisecond = uint16(bitor(bitshift(uint16(bytesDate(10)),8),uint16(bytesDate(11))));

    % Number of IDs
    bytesNumIDs = uint8(fread(fp, 4));
    if(4 ~= numel(bytesNumIDs))
        fclose(fp);
        error('Invalid header of file: "%s"',filename);
    end
    numIDs = uint32(bitor(bitor(bitshift(uint32(bytesNumIDs(1)),24),bitshift(uint32(bytesNumIDs(2)),16)),bitor(bitshift(uint32(bytesNumIDs(3)),8),uint32(bytesNumIDs(4)))));

    % All IDs (zero-terminated string per ID)
    info.listOfIDs = cell.empty();
    for i = uint32(1):numIDs
        strID = char.empty();
        while(true)
            byte = uint8(fread(fp, 1));
            if(1 ~= numel(byte))
                fclose(fp);
                error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!',numel(byte),filename);
            end
            if(~byte)
                break;
            end
            strID = append(strID, char(byte));
        end
        info.listOfIDs{i} = strID;
    end

    % Version (zero-terminated string)
    info.version = char.empty();
    while(true)
        byte = uint8(fread(fp, 1));
        if(1 ~= numel(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!',numel(byte),filename);
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
        if(1 ~= numel(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!',numel(byte),filename);
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
        if(1 ~= numel(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!',numel(byte),filename);
        end
        if(~byte)
            break;
        end
        info.dateOfCompilation = [info.dateOfCompilation, char(byte)];
    end

    % compilerVersion (zero-terminated string)
    info.compilerVersion = char.empty();
    while(true)
        byte = uint8(fread(fp, 1));
        if(1 ~= numel(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!',numel(byte),filename);
        end
        if(~byte)
            break;
        end
        info.compilerVersion = [info.compilerVersion, char(byte)];
    end

    % OS (zero-terminated string)
    info.operatingSystem = char.empty();
    while(true)
        byte = uint8(fread(fp, 1));
        if(1 ~= numel(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!',numel(byte),filename);
        end
        if(~byte)
            break;
        end
        info.operatingSystem = [info.operatingSystem, char(byte)];
    end

    % OSInfo (zero-terminated string)
    info.operatingSystemInfo = char.empty();
    while(true)
        byte = uint8(fread(fp, 1));
        if(1 ~= numel(byte))
            fclose(fp);
            error('Tried to read 1 byte but could only read %d byte(s). File: "%s" may not be complete!',numel(byte),filename);
        end
        if(~byte)
            break;
        end
        info.operatingSystemInfo = [info.operatingSystemInfo, char(byte)];
    end

    fclose(fp);
    fprintf('\b\b\b\b: OK\n');
end

