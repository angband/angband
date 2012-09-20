-- Event handler code

-- Table of event handlers
event_handlers = {}


-- Add a new event handler
function add_event_handler(event_type, handler)
	local queue = event_handlers[event_type]

	-- Create a new queue if necessary
	if not queue then
		queue = {}
		event_handlers[event_type] = queue
	end

	-- Append the handler to the queue
	table.insert(queue, 1, handler)
end


-- Remove an event handler
function remove_event_handler(event_type, handler)
	local queue = event_handlers[event_type]

	if queue then
		for i = table.getn(queue), 1, -1 do
			if queue[i] == handler then
				table.remove(queue, i)
				break
			end
		end
	end
end


-- Call a queue of event handlers
--
-- Up to 10 arguments are passed as normal function arguments,
-- if there are more then a table is used for the extra arguments.
function notify_event_hook(event_type, arg1, arg2, arg3, arg4, arg5, arg6,
                           arg7, arg8, arg9, arg10, ...)
	local queue = event_handlers[event_type]

	if queue then
		for i, handler in ipairs(queue) do
			local result

			-- HACK - Up to 10 results are handled.
			local res1, res2, res3, res4, res5, res6, res7, res8, res9, res10 =
			    handler(arg1, arg2, arg3, arg4, arg5, arg6,
			            arg7, arg8, arg9, arg10, arg)

			-- Stop handling the event queue when we get a result
			if res1 or res2 or res3 or res4 or res5 or res6 or res7 or res8 or res9 or res10 then
				return res1, res2, res3, res4, res5, res6, res7, res8, res9, res10
			end
		end
	end
end


-- Wait over several calls of an event handler
--
-- Waits for 'amount' number of event calls before resuming an
-- event-handler coroutine.
--
-- Note: This function should only be called from coroutines!
function event_wait(amount)
	local x = 0

	while x < amount do
		x = x + 1
		coroutine.yield()
	end
end
