
def GetAnimPlayRate (Location1, Time1, Location2, Time2):

    TargetAnimSpeed = 250
    PlayerScale = 0.4225

    Distance = Location2 - Location1
    TimeDifference = Time2 - Time1
    if TimeDifference == 0:
        return 0
    
    AnimSpeed = Distance / TimeDifference
    print(f"Animation Speed: {AnimSpeed}")
    AnimPlayRate = TargetAnimSpeed / (AnimSpeed * PlayerScale)
    print(f"PlayRate for TargetSpeed: {AnimPlayRate}")

    return AnimPlayRate

# Example usage

Location1   = -70
Time1       = 0.33
Location2   = 120
Time2       = 0.89
PlayRate = GetAnimPlayRate(Location1, Time1, Location2, Time2)