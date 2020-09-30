USE [master]
GO
/****** Object:  Database [vk]    Script Date: 18.01.2020 21:03:23 ******/
CREATE DATABASE [vk]
 CONTAINMENT = NONE
 ON  PRIMARY 
( NAME = N'vk', FILENAME = N'D:\Papka\myProgramms\MSSQL14.SQLEXPRESS\MSSQL\DATA\vk.mdf' , SIZE = 73728KB , MAXSIZE = UNLIMITED, FILEGROWTH = 65536KB )
 LOG ON 
( NAME = N'vk_log', FILENAME = N'D:\Papka\myProgramms\MSSQL14.SQLEXPRESS\MSSQL\DATA\vk_log.ldf' , SIZE = 204800KB , MAXSIZE = 2048GB , FILEGROWTH = 65536KB )
GO
ALTER DATABASE [vk] SET COMPATIBILITY_LEVEL = 140
GO
IF (1 = FULLTEXTSERVICEPROPERTY('IsFullTextInstalled'))
begin
EXEC [vk].[dbo].[sp_fulltext_database] @action = 'enable'
end
GO
ALTER DATABASE [vk] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [vk] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [vk] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [vk] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [vk] SET ARITHABORT OFF 
GO
ALTER DATABASE [vk] SET AUTO_CLOSE OFF 
GO
ALTER DATABASE [vk] SET AUTO_SHRINK OFF 
GO
ALTER DATABASE [vk] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [vk] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [vk] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [vk] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [vk] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [vk] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [vk] SET RECURSIVE_TRIGGERS OFF 
GO
ALTER DATABASE [vk] SET  DISABLE_BROKER 
GO
ALTER DATABASE [vk] SET AUTO_UPDATE_STATISTICS_ASYNC OFF 
GO
ALTER DATABASE [vk] SET DATE_CORRELATION_OPTIMIZATION OFF 
GO
ALTER DATABASE [vk] SET TRUSTWORTHY OFF 
GO
ALTER DATABASE [vk] SET ALLOW_SNAPSHOT_ISOLATION OFF 
GO
ALTER DATABASE [vk] SET PARAMETERIZATION SIMPLE 
GO
ALTER DATABASE [vk] SET READ_COMMITTED_SNAPSHOT OFF 
GO
ALTER DATABASE [vk] SET HONOR_BROKER_PRIORITY OFF 
GO
ALTER DATABASE [vk] SET RECOVERY SIMPLE 
GO
ALTER DATABASE [vk] SET  MULTI_USER 
GO
ALTER DATABASE [vk] SET PAGE_VERIFY CHECKSUM  
GO
ALTER DATABASE [vk] SET DB_CHAINING OFF 
GO
ALTER DATABASE [vk] SET FILESTREAM( NON_TRANSACTED_ACCESS = OFF ) 
GO
ALTER DATABASE [vk] SET TARGET_RECOVERY_TIME = 60 SECONDS 
GO
ALTER DATABASE [vk] SET DELAYED_DURABILITY = DISABLED 
GO
ALTER DATABASE [vk] SET QUERY_STORE = OFF
GO
USE [vk]
GO
/****** Object:  User [vk_request]    Script Date: 18.01.2020 21:03:24 ******/
CREATE USER [vk_request] FOR LOGIN [vk_request] WITH DEFAULT_SCHEMA=[dbo]
GO
/****** Object:  Table [dbo].[active_groups]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[active_groups](
	[ID] [int] NOT NULL,
	[short_name] [varchar](max) NULL,
	[is_active] [int] NOT NULL,
	[updated_on] [datetime] NULL,
PRIMARY KEY CLUSTERED 
(
	[ID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
/****** Object:  Table [dbo].[member_list]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[member_list](
	[group_id] [int] NOT NULL,
	[user_id] [int] NOT NULL,
 CONSTRAINT [PK_member_list] PRIMARY KEY CLUSTERED 
(
	[group_id] ASC,
	[user_id] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
/****** Object:  StoredProcedure [dbo].[ActivateGroup]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
create procedure [dbo].[ActivateGroup] (@short_name varchar(max))
with execute as owner
as
update active_groups
set is_active = 1
where short_name = @short_name
GO
/****** Object:  StoredProcedure [dbo].[ClearActiveGroups]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE procedure [dbo].[ClearActiveGroups]
with execute as owner
as
update active_groups
set is_active = 0
GO
/****** Object:  StoredProcedure [dbo].[GetGroupShortName]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
create procedure [dbo].[GetGroupShortName] (@ID int)
with execute as owner
as
select short_name from active_groups
where ID = @ID
GO
/****** Object:  StoredProcedure [dbo].[InsertGroup]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE procedure [dbo].[InsertGroup] @ID int, @short_name varchar(max)
with execute as owner
as
if not exists (select * from active_groups
where ID = @ID)
begin
	insert into active_groups(ID,short_name,is_active,updated_on)
	values (@ID,@short_name,0,'1900-01-01 00:00:00')
end
GO
/****** Object:  StoredProcedure [dbo].[SelectActive]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
create procedure [dbo].[SelectActive] 
as
select short_name from active_groups
where is_active = 1
GO
/****** Object:  StoredProcedure [dbo].[SelectUnion]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
create procedure [dbo].[SelectUnion]
with execute as owner
as
select user_id from (select user_id,count(*) as count from member_list
join active_groups on member_list.group_id = active_groups.ID
where active_groups.is_active = 1
group by user_id) as sel1 where count = (select count(*) from active_groups where active_groups.is_active = 1)
GO
/****** Object:  StoredProcedure [dbo].[SortedGroups]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE procedure [dbo].[SortedGroups]
with execute as owner
as
select short_name from active_groups
order by updated_on
GO
/****** Object:  StoredProcedure [dbo].[UpdateGroup]    Script Date: 18.01.2020 21:03:24 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE procedure [dbo].[UpdateGroup] @ID int
with execute as owner
as
update active_groups
set updated_on = GETDATE()
where ID = @ID
GO
USE [master]
GO
ALTER DATABASE [vk] SET  READ_WRITE 
GO
