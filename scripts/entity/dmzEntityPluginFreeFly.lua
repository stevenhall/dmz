local Right = dmz.math.right ()
local Up = dmz.math.up ()
local Forward = dmz.math.forward ()

function update_time_slice (self, time)

   local hil = dmz.object.get_human_in_the_loop ()

   if hil and self.active > 0 then

      local pos = dmz.object.position (hil)
      local vel = dmz.object.velocity (hil)
      local ori = dmz.object.orientation (hil)

      if not ori then ori = dmz.matrix.new () end
      if not vel then vel = dmz.vector.new () end

      if pos then

         local dir = ori:transform (Forward)

         local headingVec = dmz.vector.new (dir)
         headingVec:set_y (0.0)
         headingVec = headingVec:normalize ()
         
         local heading = Forward:get_angle (headingVec)
         local hcross = Forward:cross (headingVec)
         hcross = hcross:normalize ()
         
         if hcross:get_y () < 0.0 then
            heading = heading + (self.turn * time)
            heading = dmz.math.TwoPi - heading
         else heading = heading - (self.turn * time)
         end
         
         if heading > dmz.math.Pi then heading = heading - dmz.math.TwoPi
         elseif heading < -dmz.math.Pi then heading = heading + dmz.math.TwoPi
         end
         
         local pitch = dir:get_angle (headingVec)
         
         if dir:get_y () < 0.0 then
            pitch = pitch + self.pitch * time
            pitch = dmz.math.TwoPi - pitch
         else pitch = pitch - self.pitch * time
         end
         
         if pitch > dmz.math.HalfPi and pitch <= dmz.math.Pi then
            pitch = dmz.math.HalfPi - 0.001
         elseif pitch < (dmz.math.Pi + dmz.math.HalfPi) and pitch > dmz.math.Pi then
            pitch = dmz.math.HalfPi + dmz.math.Pi + 0.001
         end
         
         local pm = dmz.matrix.new ():from_axis_and_angle (Right, pitch)
         
         local hm = dmz.matrix.new ():from_axis_and_angle (Up, heading)
         
         ori = hm * pm
         
         dir = ori:transform (Forward)
         local slide = ori:transform (Right)
         
         local OldPos = pos
         
         pos = pos - (dir * (self.speed * time))
         pos = pos + (slide * (self.strafe * time))
         
         if not dmz.math.is_zero (time) then vel = (pos - OldPos) * (1 / time) end
         
         dmz.object.position (hil, nil, pos)
         dmz.object.velocity (hil, nil, vel)
         dmz.object.orientation (hil, nil, ori)
      end
   end
end


function receive_input_event (self, event)

   if event.state then 
      if event.state.active then  self.active = self.active + 1
      else self.active = self.active - 1 end

      if self.active == 1 then self.timeSlice:start (self.handle)
      elseif self.active == 0 then self.timeSlice:stop (self.handle)
      end
   end

   if event.axis then
      local value = 0
      if not dmz.math.is_zero (event.axis.value, 0.1) then
         value = event.axis.value * math.abs (event.axis.value)
      end
      
      if event.axis.which == 2 then self.speed = value * self.moveSpeed
      elseif event.axis.which == 1 then self.turn = value * self.turnRate
      elseif event.axis.which == 6 then self.strafe = value * self.moveSpeed
      elseif event.axis.which == 7 then self.pitch = value * self.turnRate
      end
   end
end


function start (self)
   self.handle = self.timeSlice:create (update_time_slice, self, self.name)

   self.obs:init_channels (
      self.config,
      dmz.input.Axis + dmz.input.ChannelState,
      receive_input_event,
      self);

   if self.handle and self.active == 0 then self.timeSlice:stop (self.handle) end
end


function stop (self)
   if self.handle and self.timeSlice then self.timeSlice:destroy (self.handle) end
   self.obs:release_all ()
end


function new (config, name)
   local self = {
      start_plugin = start,
      stop_plugin = stop,
      name = name,
      log = dmz.log.new ("lua." .. name),
      timeSlice = dmz.time_slice.new (),
      obs = dmz.input_observer.new (),
      active = 0,
      moveSpeed = config:lookup_number ("movement.speed", 1.0),
      turnRate = config:lookup_number ("movement.turnRate", 1.0),
      speed = 0.0,
      strafe = 0.0,
      turn = 0.0,
      pitch = 0.0,
      config = config,
   }
      
   self.log:info ("Creating plugin: " .. name)
   
   return self
end

