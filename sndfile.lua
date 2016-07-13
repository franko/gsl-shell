local ffi = require 'ffi'

dofile("sndfile-cdefs.lua")

local clib = ffi.load('libsndfile-1')

local SF_INFO = ffi.typeof("struct SF_INFO")

local function c_open_mode(mode)
    if mode == "r" then
        return clib.SFM_READ
    elseif mode == "w" then
        return clib.SFM_WRITE
    elseif mode == "rw" then
        return clib.SFM_RDRW
    else
        error("invalid file open mode: " .. mode)
    end
end

local function open(filename, mode)
    local c_info = SF_INFO()
    local c_sndfile = clib.sf_open(filename, c_open_mode(mode), c_info)
    local info = {
        frames     = tonumber(c_info.frames),
        samplerate = c_info.samplerate,
        channels   = c_info.channels,
        format     = c_info.format,
        sections   = c_info.sections,
        seekable   = (c_info.seekable ~= 0),
    }
    local buffer = ffi.new("double[?]", info.channels)
    local function read()
        local count = clib.sf_readf_double(c_sndfile, buffer, 1)
        if count == 1 then
            local frame = {}
            for k = 1, info.channels do
                frame[k] = buffer[k - 1]
            end
            return frame
        end
    end
    local function close()
        clib.sf_close(c_sndfile)
    end
    local sndfile = {
        read = read,
        close = close,
    }
    return sndfile, info
end

return {open = open}
