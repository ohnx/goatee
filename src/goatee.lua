--- Template Engine.
--
-- Takes a string with embedded Lua code block and renders
-- it based on the content of the blocks.
--
-- All template blocks start with '{ + start modifier' and 
-- end with 'end modifier + }'.
--
-- Supports:
--  * {# text #} for comments.
--  * {% func %} for running Lua code.
--  * {{ var }}  for printing variables.
--
-- Use \{ to use a literal { in the template if you need a literal {.
-- Also a { without an end modifier following will be treated as literal {.
--
-- Template block ends that end a line (whether they are part of a valid 
-- block or not) will not create a new line. Use a space ' ' at the end
-- of the line if you want a new line preserved. The space will be removed.
-- So use two if you want the newline and the space preserved.
--  
-- Multi-line strings in Lua blocks are supported but
-- [[ is not allowed. Use [=[ or some other variation.
--
-- Both compile and compile_file can take an optional
-- env table which when provided will be used as the
-- env for the Lua code in the template. This allows
-- a level of sandboxing. Note that any globals including
-- libraries that the template needs to access must be
-- provided by env if used.
  
local M = {}
 
-- Note: Modifiers and end modifiers must be symbols.
 
--- Map of start block modifiers to their end block modifier.
local END_MODIFIER = {
    ["#"] = "#",
    ["%"] = "%",
    ["{"] = "}",
}
 
--- Actions that should be taken when a block is encountered.
local MODIFIER_FUNC = {
    ["#"] = function(code)
        return ""
    end,
 
    ["%"] = function(code)
        return code
    end,
 
    ["{"] = function(code)
        return ("_ret[#_ret+1] = %s"):format(code)
    end,
}
 
--- Handle newline rules for blocks that end a line.
-- Blocks ending with a space keep their newline and blocks that
-- do not lose their newline.
local function handle_block_ends(text)
    local modifier_set = ""
 
    -- Build up the set of end modifiers.
    -- Prefix each modifier with % to ensure they are escaped properly for gsub.
    -- Block ends are and must always be symbols.
    for _,v in pairs(END_MODIFIER) do
        modifier_set = modifier_set.."%"..v
    end
 
    text = text:gsub("(["..modifier_set.."])} \n", "%1}\n\n")
    text = text:gsub("(["..modifier_set.."])}\n", "%1}")
 
    return text
end
  
--- Append text or code to the builder.
local function appender(builder, text, code)
    if code then
        builder[#builder+1] = code
    elseif text then
        -- [[ has a \n immediately after it. Lua will strip
        -- the first \n so we add one knowing it will be
        -- removed to ensure that if text starts with a \n
        -- it won't be lost.
        builder[#builder+1] = "_ret[#_ret+1] = [[\n" .. text .. "]]"
    end
end
  
--- Takes a string and determines what kind of block it
-- is and takes the appropriate action.
--
-- The text should be something like:
-- "{{ ... }}"
-- 
-- If the block is supported the begin and end tags will
-- be stripped and the associated action will be taken.
-- If the tag isn't supported the block will be output
-- as is.
local function run_block(builder, text)
    local func
    local modifier
  
     -- Text is {...
     -- Pull out the character after { to determine if we
     -- have a modifier and what action needs to be taken.
    modifier = text:sub(2, 2)
  
    func = MODIFIER_FUNC[modifier]
    if func then
        appender(builder, nil, func(text:sub(3, #text-3)))
    else
        appender(builder, text)
    end
end
  
--- Compile a Lua template into a string.
--
-- @param      tmpl The template.
-- @param[opt] env  Environment table to use for sandboxing.
--
-- return Compiled template.
function M.compile(tmpl, env)
    -- Turn the template into a string that can be run though
    -- Lua. Builder will be used to efficiently build the string
    -- we'll run. The string will use it's own builder (_ret). Each
    -- part that comprises _ret will be the various pieces of the
    -- template. Strings, variables that should be printed and
    -- functions that should be run.
    local builder = { "_ret = {}\n" }
    local pos     = 1
    local b
    local modifier
    local ret
    local func
    local err
    local out
  
    if #tmpl == 0 then
        return ""
    end
 
    -- Handle the new line rules for block ends.
    tmpl = handle_block_ends(tmpl)
 
    while pos < #tmpl do
        -- Look for start of a block.
        b = tmpl:find("{", pos)
        if not b then
            break
        end
  
        -- Check if this is a block or escaped { or not followed by block modifier.
        -- We store the next character as the modifier to help us determine if 
        -- we have encountered a block or not.
        modifier = tmpl:sub(b+1, b+1)
        if tmpl:sub(b-1, b-1) == "\\" then
            appender(builder, tmpl:sub(pos, b-2))
            appender(builder, "{")
            pos = b+1
        elseif not END_MODIFIER[modifier] then
            appender(builder, tmpl:sub(pos, b+1))
            pos = b+2
        else
            -- Some modifiers for block ends aren't the same as the block start modifier.
            modifier = END_MODIFIER[modifier]
            -- Add all text up until this block.
            appender(builder, tmpl:sub(pos, b-1))
            -- Find the end of the block.
            pos = tmpl:find(("[^\\]?%s}"):format(modifier), b)
            if pos then
                -- If we captured a character before the modifier move past it.
                if tmpl:sub(pos, pos) ~= modifier then
                    pos = pos+1
                end
                run_block(builder, tmpl:sub(b, pos+2))
                -- Skip past the *} (pos points to the start of *}).
                pos = pos+2
            else
                -- Add back the { because we don't have an end block.
                -- We want to keep any text that isn't in a real block.
                appender(builder, "{")
                pos = b+1
            end
        end
    end
    -- Add any text after the last block. Or all of it if there
    -- are no blocks.
    if pos then
        appender(builder, tmpl:sub(pos, #tmpl-1))
    end
  
     -- Create the compiled template.
    builder[#builder+1] = "return table.concat(_ret)"
 
    -- Run the Lua code we built though Lua and get the result.
    ret, func, err = pcall(load, table.concat(builder, "\n"), "template", "t", env)
    if not ret then
        return nil, func
    end
    if not func then
        return nil, err
    end
    ret, out, err = pcall(func)
    if not ret then
        return nil, out
    end
    if not out then
        return nil, err
    end
    return out
end
  
--- Compile a Lua template into a string by
-- reading the template from a file.
--
-- @param      name The file name to read from.
-- @param[opt] env  Environment table to use for sandboxing.
--
-- return Compiled template.
function M.compile_file(name, env)
    local f, err = io.open(name, "rb")
    if not f then
        return err
    end
    local t = f:read("*all")
    f:close()
    return M.compile(t, env)
end
  
return M