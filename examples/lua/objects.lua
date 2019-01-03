function classBinding(name)
  person = Employee.new(name, 21, 1000)
  print(handle:getPerson():getName() .. ' is ' .. handle:getPerson():getAge() .. ' years old')
  handle:getPerson():setName(name)
  print(handle:getPerson():getName() .. ' earns ' .. person:getSalary())
end